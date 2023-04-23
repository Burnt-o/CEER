#include "pch.h"
#include "MapReader.h"
#include "MultilevelPointer.h"
#include "PointerManager.h"

// A lot of the reverse engineering here was already done by the assembly guys
//https://github.com/XboxChaos/Assembly/blob/9c2aabd70b1d40fedc02942fca888b71f940ce10/src/Blamite/Formats/Halo1/Layouts/H1_Layouts_Core.xml

struct mapHeader {
    uint32_t tagTableOffset;
    datum scenarioDatum;
    // Then a bunch of other stuff I don't care about
};




struct tagElement { 
    uint32_t tagGroupMagic;
    uint32_t parentGroupMagic;
    uint32_t grandparentGroupMagic;
    datum tagDatum;
    uint32_t nameOffset;
    uint32_t offset;
    uint32_t isInDataFile;
    uint32_t pad;
};
static_assert(sizeof(tagElement) == 0x20);



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
    uint32_t tagDataBase = 0x50000000;
    MCCString* objectNameTable;




    // handle to our callback of LevelLoadHook so we can remove it in destructor
    eventpp::CallbackList<void(HaloLevel)>::Handle mLevelLoadCallbackHandle = {};
    eventpp::CallbackList<void(HaloLevel)>& mLevelLoadEvent;

    tagElement* getTagElement(datum tagDatum);
    uintptr_t getTagAddress(datum tagDatum);
    uintptr_t getTagAddress(uint32_t tagOffset);



    std::once_flag lazyInitOnceFlag;
    void lazyInit();

public:
    explicit MapReaderImpl(eventpp::CallbackList<void(HaloLevel)>& levelLoadEvent);
    ~MapReaderImpl();

    actorPaletteWrapper getActorPalette();
    bipedPaletteWrapper getBipedPalette();
    std::string getTagName(tagReference* tag);

    bipedTagReference* getActorsBiped(actorTagReference* tag);
    faction getBipedFaction(bipedTagReference* tag);

    std::string getObjectName(int nameIndex);

    void onLevelLoadEvent(HaloLevel newLevel);     // What we run when new level is loaded changes
};

MapReader::MapReader(eventpp::CallbackList<void(HaloLevel)>& levelLoadEvent) : impl(new MapReaderImpl(levelLoadEvent)) {}
MapReader::~MapReader() = default; // https://www.fluentcpp.com/2017/09/22/make-pimpl-using-unique_ptr/



MapReader::MapReaderImpl::MapReaderImpl(eventpp::CallbackList<void(HaloLevel)>& levelLoadEvent) : mLevelLoadEvent(levelLoadEvent)
{
    mLevelLoadCallbackHandle = levelLoadEvent.append([this](HaloLevel a) { this->onLevelLoadEvent(a); });
    assert(reverseStringMagic("actv") == 0x61637476);
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

faction MapReader::MapReaderImpl::getBipedFaction(bipedTagReference* tag)
{
    constexpr int defaultTeamOffset = 0x180;
    uint8_t fac = *(uint8_t*)(getTagAddress(tag->tagDatum) + defaultTeamOffset);
    return (faction)fac;

}





bipedTagReference* MapReader::MapReaderImpl::getActorsBiped(actorTagReference* tag)
{
    constexpr int bipedTagReferenceOffset = 0x14; 
    uintptr_t actorTag = getTagAddress(tag->tagDatum);
    uintptr_t bipedRef = actorTag + bipedTagReferenceOffset;

    if (IsBadReadPtr((void*)bipedRef, sizeof(bipedTagReference))) throw CEERRuntimeException(std::format("getActorsBiped got bad memory, actorTag: {:#X}, bipedRef: {:#X}", actorTag, bipedRef));

    bipedTagReference* biped = (bipedTagReference*)bipedRef;
    // test that the magic is correct

    PLOG_VERBOSE << "testing : stringMagic(vtca): " << std::hex << stringMagic("vtca");
    PLOG_VERBOSE << "testing : reverseStringMagic(vtca): " << std::hex << reverseStringMagic("vtca");
    PLOG_VERBOSE << "(should be :" << std::hex << 0x61637476 << ")";


    if (biped->tagGroupMagic != reverseStringMagic("bipd"))
    {
        PLOG_ERROR << "actv has no bipd tag!";
        return nullptr; // this is actually somethign that can happen, not all actv's have a bipd ref
    }


    return biped;
}

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

uintptr_t MapReader::MapReaderImpl::getTagAddress(datum tagDatum)
{
    return getTagAddress(getTagElement(tagDatum)->offset);
}

tagElement* MapReader::MapReaderImpl::getTagElement(datum tagDatum)
{
    mapHeader* header = reinterpret_cast<mapHeader*>(currentCacheAddress);
    if (IsBadReadPtr(header, 16)) throw CEERRuntimeException("mapHeader bad address");
    int tagTableOffset = header->tagTableOffset - tagDataBase;

    PLOG_VERBOSE << "tagTableOffset: " << tagTableOffset;
    PLOG_VERBOSE << "tagDatum.index: " << tagDatum.index;
    uintptr_t tagEle = currentCacheAddress + tagTableOffset + (tagDatum.index * sizeof(tagElement));

    if (IsBadReadPtr((void*)tagEle, sizeof(tagElement))) throw CEERRuntimeException(std::format("bad tagElement read, invalid memory: tagDatum.index: {:#X}, address: {:#X}", tagDatum.index, (uint64_t)tagEle));

    tagElement* tagElem = (tagElement*)tagEle;
    if (tagDatum.salt != tagElem->tagDatum.salt) throw CEERRuntimeException(std::format("bad tagElement read, mismatching datum data: tagDatum.index: {:#X}, tagElem.index: {:#X}, address: {:#X}", tagDatum.index, tagElem->tagDatum.index, (uint64_t)tagEle));

    PLOG_VERBOSE << std::format("reading tagElement from {:#X}, datum: {:#X}{:#X}", tagEle, tagElem->tagDatum.salt, tagElem->tagDatum.index);
    return (tagElement*)tagEle;
}


std::string MapReader::MapReaderImpl::getTagName(tagReference* tag)
{
    return std::string((char*)getTagAddress(tag->nameOffset));
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

std::string MapReader::getTagName(tagReference* tag) { return impl.get()->getTagName(tag); }

bipedTagReference* MapReader::getActorsBiped(actorTagReference* tag) { return impl.get()->getActorsBiped(tag); }

faction MapReader::getBipedFaction(bipedTagReference* tag) { return impl.get()->getBipedFaction(tag); }

std::string MapReader::getObjectName(int nameIndex) { return impl.get()->getObjectName(nameIndex); }



 uint32_t MapReader::stringMagic(std::string str)
{
    if (str.length() != 4) throw CEERRuntimeException(std::format("stringMagic bad string length: {}", str.length()));
    uint32_t out;
    //unsigned char* p = (unsigned char*)str.c_str();
    //out = p[0] + 256U * p[1] + 65536U * p[2] + 16777216U * p[3];

    out = *(uint32_t*)str.c_str();

}


 uint32_t MapReader::reverseStringMagic(std::string str)
{
    reverse(str.begin(), str.end());
    return stringMagic(str);
}


