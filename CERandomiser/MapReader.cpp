#include "pch.h"
#include "MapReader.h"
#include "MultilevelPointer.h"
#include "PointerManager.h"

// A lot of the reverse engineering here was already done by the assembly guys
//https://github.com/XboxChaos/Assembly/blob/9c2aabd70b1d40fedc02942fca888b71f940ce10/src/Blamite/Formats/Halo1/Layouts/H1_Layouts_Core.xml


bool datum::operator<(const datum& rhs) const
{
    return index < rhs.index;
}

bool datum::operator==(const datum& rhs) const
{
    return salt == rhs.salt;
}

constexpr uint32_t tagDataBase = 0x50000000;

struct mapHeader {
private:
    uint32_t mTagTableOffset;

public:
    datum scenarioDatum;
    uint32_t checksum;
    uint32_t numberOfTags;



    uint32_t tagTableOffset() { return mTagTableOffset - tagDataBase; }
    // Then a bunch of other stuff I don't care about
};








struct tagBlock {
    uint32_t entryCount;
       uint32_t pointer;
       uint32_t blockDefinition;
};
static_assert(sizeof(tagBlock) == 0xC);

constexpr int tagSize = 0x10;




class MapReader::MapReaderImpl {


private:
    std::mutex mDestructionGuard; // Protects against destruction while callbacks are executing

    // Data
    std::shared_ptr<MultilevelPointer> mlp_currentCache;
    //std::shared_ptr<MultilevelPointer> mlp_tagDataBase;
    uintptr_t currentCacheAddress;
    uintptr_t scenarioAddress;

    MCCString* objectNameTable;




    // handle to our callback of LevelLoadHook so we can remove it in destructor
    eventpp::CallbackList<void(HaloLevel)>::Handle mLevelLoadCallbackHandle = {};
    eventpp::CallbackList<void(HaloLevel)>& mLevelLoadEvent;

    
    uintptr_t getTagAddress(const datum& tagDatum);
    uintptr_t getTagAddress(uint32_t tagOffset);



    std::once_flag lazyInitOnceFlag;
    void lazyInit();

public:
    explicit MapReaderImpl(eventpp::CallbackList<void(HaloLevel)>& levelLoadEvent);
    ~MapReaderImpl();

    tagElement* getTagElement(const datum& tagDatum);

    actorPaletteWrapper getActorPalette();
    bipedPaletteWrapper getBipedPalette();
    std::string getTagName(const datum& tagDatum);

    //datum getActorsBiped(const datum& actorDatum);
    faction getBipedFaction(const datum& bipedDatum);
    faction getActorsFaction(const datum& actorDatum);
    std::string getObjectName(int nameIndex);

    std::span<tagElement> getTagTable();

    void onLevelLoadEvent(HaloLevel newLevel);     // What we run when new level is loaded changes
};

MapReader::MapReader(eventpp::CallbackList<void(HaloLevel)>& levelLoadEvent) : impl(new MapReaderImpl(levelLoadEvent)) {}
MapReader::~MapReader() = default; // https://www.fluentcpp.com/2017/09/22/make-pimpl-using-unique_ptr/



MapReader::MapReaderImpl::MapReaderImpl(eventpp::CallbackList<void(HaloLevel)>& levelLoadEvent) : mLevelLoadEvent(levelLoadEvent)
{
    mLevelLoadCallbackHandle = levelLoadEvent.append([this](HaloLevel a) { this->onLevelLoadEvent(a); });
    assert(stringToMagic("actv") == 0x61637476);
}


void MapReader::MapReaderImpl::lazyInit()
{
    try
    {
        mlp_currentCache = PointerManager::getMultilevelPointer("currentCacheAddress");
        //mlp_tagDataBase = PointerManager::getMultilevelPointer("tagDataBase");
    }
    catch (InitException& ex)
    {
        ex.prepend("MapReader setup failed: ");
        throw CEERRuntimeException(ex.what());
    }
}

MapReader::MapReaderImpl::~MapReaderImpl()
{
    std::scoped_lock<std::mutex> lock(mDestructionGuard);
    mLevelLoadEvent.remove(mLevelLoadCallbackHandle);

}


faction MapReader::MapReaderImpl::getActorsFaction(const datum& actorDatum)
{
        constexpr int bipedTagReferenceOffset = 0x14; 
        uintptr_t actorTag = getTagAddress(actorDatum);
        uintptr_t bipedRef = actorTag + bipedTagReferenceOffset;
    
        if (IsBadReadPtr((void*)bipedRef, sizeof(bipedTagReference))) throw CEERRuntimeException(std::format("getActorsBiped got bad memory, actorTag: {:#X}, bipedRef: {:#X}", actorTag, bipedRef));
    
            tagReference* biped = (tagReference*)bipedRef;
    
          
        // test that the magic is correct
        if (biped->tagGroupMagic != stringToMagic("bipd") || biped->tagDatum == nullDatum)
        {
            PLOG_ERROR << "actv has no bipd tag!";
            return faction::Undefined; // this is actually somethign that can happen, not all actv's have a bipd ref
        }
    


        // lookup the bipeds faction
        return getBipedFaction(biped->tagDatum);
        
}

faction MapReader::MapReaderImpl::getBipedFaction(const datum& bipedDatum)
{
    constexpr int defaultTeamOffset = 0x180;
    uint8_t fac = *(uint8_t*)(getTagAddress(bipedDatum) + defaultTeamOffset);
    return (faction)fac;

}

//
//
//
//
//datum MapReader::MapReaderImpl::getActorsBiped(const datum& actorDatum)
//{
//    constexpr int bipedTagReferenceOffset = 0x14; 
//    uintptr_t actorTag = getTagAddress(actorDatum);
//    uintptr_t bipedRef = actorTag + bipedTagReferenceOffset;
//
//    if (IsBadReadPtr((void*)bipedRef, sizeof(bipedTagReference))) throw CEERRuntimeException(std::format("getActorsBiped got bad memory, actorTag: {:#X}, bipedRef: {:#X}", actorTag, bipedRef));
//
//        tagReference* biped = (tagReference*)bipedRef;
//
//      
//    // test that the magic is correct
//
//    if (biped->tagGroupMagic != stringToMagic("bipd"))
//    {
//        PLOG_ERROR << "actv has no bipd tag!";
//        return nullDatum; // this is actually somethign that can happen, not all actv's have a bipd ref
//    }
//
//
//    return biped->tagDatum;
//}

actorPaletteWrapper MapReader::MapReaderImpl::getActorPalette()
{
    constexpr int actorPaletteReferenceOffset = 0x420; // relative to scenario tag
    tagBlock* actorPalleteRef = (tagBlock*)(scenarioAddress + actorPaletteReferenceOffset);
    actorPaletteWrapper out;
    out.tagCount = actorPalleteRef->entryCount;
    out.firstTag = (actorTagReference*)getTagAddress(actorPalleteRef->pointer);
    return out;
}

bipedPaletteWrapper MapReader::MapReaderImpl::getBipedPalette()
{
    constexpr int bipedPaletteReferenceOffset = 0x234; // relative to scenario tag
    tagBlock* bipedPalleteRef = (tagBlock*)(scenarioAddress + bipedPaletteReferenceOffset);
    bipedPaletteWrapper out;
    out.tagCount = bipedPalleteRef->entryCount;
    out.firstTag = (bipedTagReference*)getTagAddress(bipedPalleteRef->pointer);
    return out;
}



uintptr_t MapReader::MapReaderImpl::getTagAddress(uint32_t offset)
{
    //PLOG_VERBOSE << "offset: " << std::hex << offset;
    //PLOG_VERBOSE << "offset - tagDataBase: " << std::hex << (offset - tagDataBase);

    if (offset < tagDataBase) throw CEERRuntimeException(std::format("bad tagAddress offset, offset: {:#X}", offset));
    uintptr_t out = currentCacheAddress + (offset - tagDataBase);
        if (IsBadReadPtr((void*)out, 16)) throw CEERRuntimeException(std::format("bad tagAddress read, offset: {:#X}, offset - tagDataBase: {:#X}, address: {:#X}", offset, (offset - tagDataBase), (uint64_t)out));
    return out;
}

uintptr_t MapReader::MapReaderImpl::getTagAddress(const datum& tagDatum)
{
    return getTagAddress(getTagElement(tagDatum)->offset);
}

tagElement* MapReader::MapReaderImpl::getTagElement(const datum& tagDatum)
{
    mapHeader* header = reinterpret_cast<mapHeader*>(currentCacheAddress);
    if (IsBadReadPtr(header, 16)) throw CEERRuntimeException("mapHeader bad address");

    PLOG_VERBOSE << "tagTableOffset: " << header->tagTableOffset();
    PLOG_VERBOSE << "tagDatum.index: " << tagDatum.index;
    uintptr_t tagEle = currentCacheAddress + header->tagTableOffset() + (tagDatum.index * sizeof(tagElement));

    if (IsBadReadPtr((void*)tagEle, sizeof(tagElement))) throw CEERRuntimeException(std::format("bad tagElement read, invalid memory: tagDatum.index: {:#X}, address: {:#X}", tagDatum.index, (uint64_t)tagEle));

    tagElement* tagElem = (tagElement*)tagEle;
    if (tagDatum.salt != tagElem->tagDatum.salt) throw CEERRuntimeException(std::format("bad tagElement read, mismatching datum data: tagDatum.index: {:#X}, tagElem.index: {:#X}, address: {:#X}", tagDatum.index, tagElem->tagDatum.index, (uint64_t)tagEle));

    PLOG_VERBOSE << std::format("reading tagElement from {:#X}, datum: {:#X}{:#X}", tagEle, tagElem->tagDatum.salt, tagElem->tagDatum.index);
    return (tagElement*)tagEle;
}


std::string MapReader::MapReaderImpl::getTagName(const datum& tagDatum)
{
    return std::string((char*)getTagAddress(getTagElement(tagDatum)->nameOffset));
}


std::string MapReader::MapReaderImpl::getObjectName(int nameIndex)
{
    if (nameIndex < 0 || nameIndex > 0xFFFF) return "";
    return (objectNameTable + nameIndex)->copy();
}

// We want to load data relavent to the currently loaded cache file
void MapReader::MapReaderImpl::onLevelLoadEvent(HaloLevel newLevel)
{

    std::scoped_lock<std::mutex> lock(mDestructionGuard);
    PLOG_VERBOSE << "MapReader::MapReaderImpl::onLevelLoadEvent";
    std::call_once(lazyInitOnceFlag, [this]() {lazyInit(); });

    if (!mlp_currentCache.get()->resolve(&currentCacheAddress)) throw CEERRuntimeException(std::format("Could not resolve currentCacheAddress, {}, {:#X}", MultilevelPointer::GetLastError(), (uint64_t)currentCacheAddress));
    PLOG_VERBOSE << "currentCacheAddress: " << std::hex << currentCacheAddress;
    mapHeader* header = (mapHeader*)currentCacheAddress;


    tagElement* scenarioTagElement = (tagElement*)getTagElement(header->scenarioDatum);
    PLOG_VERBOSE << "scenarioTagElement: " << std::hex << scenarioTagElement;

    scenarioAddress = getTagAddress(scenarioTagElement->offset);
    PLOG_VERBOSE << "scenarioAddress: " << std::hex << scenarioAddress;
    if (IsBadReadPtr((void*)scenarioAddress, 16)) throw CEERRuntimeException("scenarioAddress not resolved");
    if (*(char*)scenarioAddress != 0x70) throw CEERRuntimeException("scenarioAddress bad address");

    auto objectNameTableTagBlock = (tagBlock*)(scenarioAddress + 0x204);
    objectNameTable = (MCCString*)getTagAddress(objectNameTableTagBlock->pointer);

}











actorPaletteWrapper MapReader::getActorPalette() { return impl.get()->getActorPalette(); }
bipedPaletteWrapper MapReader::getBipedPalette() { return impl.get()->getBipedPalette(); }

std::string MapReader::getTagName(const datum& tagDatum) { return impl.get()->getTagName(tagDatum); }

//datum MapReader::getActorsBiped(const datum& actorDatum) { return impl.get()->getActorsBiped(actorDatum); }

faction MapReader::getBipedFaction(const datum& bipedDatum) { return impl.get()->getBipedFaction(bipedDatum); }

std::string MapReader::getObjectName(int nameIndex) { return impl.get()->getObjectName(nameIndex); }

std::span<tagElement> MapReader::getTagTable() { return impl.get()->getTagTable(); }

tagElement* MapReader::getTagElement(const datum& tagDatum) { return impl.get()->getTagElement(tagDatum); }

faction MapReader::getActorsFaction(const datum& actorDatum) { return impl.get()->getActorsFaction(actorDatum); }

 uint32_t MapReader::stringToMagic(std::string str)
{
    reverse(str.begin(), str.end());
    if (str.length() != 4) throw CEERRuntimeException(std::format("stringMagic bad string length: {}", str.length()));
    uint32_t out;
    //unsigned char* p = (unsigned char*)str.c_str();
    //out = p[0] + 256U * p[1] + 65536U * p[2] + 16777216U * p[3];

    out = *(uint32_t*)str.c_str();

}


 std::string MapReader::magicToString(uint32_t magic)
{
     std::string str(reinterpret_cast<char*>(&magic), sizeof(uint32_t));
     reverse(str.begin(), str.end());
     return str;
}




 std::span<tagElement> MapReader::MapReaderImpl::getTagTable()
 {
    mapHeader* header = (mapHeader*)currentCacheAddress;

    uintptr_t pTagTable = currentCacheAddress + header->tagTableOffset();
    if (IsBadReadPtr((void*)pTagTable, sizeof(tagElement))) throw CEERRuntimeException(std::format("Could not resolve tagTable address, cacheAddress: {:X}, tagTableOffset: {:X}, tagTable: {:X}", currentCacheAddress, header->tagTableOffset(), (currentCacheAddress + header->tagTableOffset())));

    tagElement* startTagTable = (tagElement*)pTagTable;
    tagElement* endTagTable = startTagTable + header->numberOfTags;

    PLOG_DEBUG << "tag table start: " << std::hex << (void*)startTagTable;
    if (IsBadReadPtr((void*)endTagTable, sizeof(tagElement))) throw CEERRuntimeException("Could not resolve end of tagTable address");
    
    std::span<tagElement> out{ startTagTable, header->numberOfTags };
    return out;
    
 
 }
