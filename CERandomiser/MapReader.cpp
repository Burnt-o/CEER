#include "pch.h"
#include "MapReader.h"
#include "MultilevelPointer.h"
#include "PointerManager.h"

// A lot of the reverse engineering here was already done by the assembly guys
//https://github.com/XboxChaos/Assembly/blob/9c2aabd70b1d40fedc02942fca888b71f940ce10/src/Blamite/Formats/Halo1/Layouts/H1_Layouts_Core.xml

static_assert(MapReader::stringToMagic("actv") == 0x61637476);
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


struct rangef {
    float min;
    float max;
};
static_assert(sizeof(rangef) == 0x8);






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
    tagBlock* actorPalette;




    std::once_flag lazyInitOnceFlag;
    void lazyInit();

public:
    explicit MapReaderImpl() = default;
    ~MapReaderImpl() { std::scoped_lock<std::mutex> lock(mDestructionGuard); }

    tagElement* getTagElement(const datum& tagDatum);
    uintptr_t getTagAddress(const datum& tagDatum);
    uintptr_t getTagAddress(uint32_t tagOffset);

    tagBlock* getActorPalette() { return actorPalette; }
    //tagBlock* getBipedPalette();
    std::string getTagName(const datum& tagDatum);

    //datum getActorsBiped(const datum& actorDatum);
    faction getBipedFaction(const datum& bipedDatum);
    faction getActorsFaction(const datum& actorDatum);
    std::string getObjectName(int nameIndex);
    datum getEncounterSquadDatum(int encounterIndex, int squadIndex);

    std::span<tagElement> getTagTable();

    void cacheTagData(HaloLevel newLevel);     // What we run when new level is loaded changes
};

MapReader::MapReader() : impl(new MapReaderImpl()) {}
MapReader::~MapReader() = default; // https://www.fluentcpp.com/2017/09/22/make-pimpl-using-unique_ptr/






void MapReader::MapReaderImpl::lazyInit()
{
    try
    {
        mlp_currentCache = PointerManager::getMultilevelPointer("currentCacheAddress");
        //mlp_tagDataBase = PointerManager::getMultilevelPointer("tagDataBase");
    }
    catch (InitException& ex) // convert initException to runtime exception since this might happen at any time
    {
        ex.prepend("MapReader setup failed: ");
        throw CEERRuntimeException(ex.what());
    }
}

// We want to load data relavent to the currently loaded cache file
void MapReader::MapReaderImpl::cacheTagData(HaloLevel newLevel)
{
    std::scoped_lock<std::mutex> lock(mDestructionGuard);
    PLOG_VERBOSE << "MapReader::MapReaderImpl::cacheTagData";
    std::call_once(lazyInitOnceFlag, [this]() {lazyInit(); }); // flag not flipped if exception thrown

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

    constexpr int actorPaletteReferenceOffset = 0x420; // relative to scenario tag
    this->actorPalette = (tagBlock*)(scenarioAddress + actorPaletteReferenceOffset);
    PLOG_DEBUG << "actorPalette ptr" << std::hex << (uintptr_t)this->actorPalette;

}



faction MapReader::MapReaderImpl::getActorsFaction(const datum& actorDatum)
{
        constexpr int bipedTagReferenceOffset = 0x14; 
        uintptr_t actorTag = getTagAddress(actorDatum);
        uintptr_t bipedRef = actorTag + bipedTagReferenceOffset;
    
        if (IsBadReadPtr((void*)bipedRef, 0x30)) throw CEERRuntimeException(std::format("getActorsBiped got bad memory, actorTag: {:#X}, bipedRef: {:#X}", actorTag, bipedRef));
    
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



//tagBlock* MapReader::MapReaderImpl::getBipedPalette()
//{
//    constexpr int bipedPaletteReferenceOffset = 0x234; // relative to scenario tag
//    tagBlock* bipedPalleteRef = (tagBlock*)(scenarioAddress + bipedPaletteReferenceOffset);
//    bipedPaletteWrapper out;
//    out.tagCount = bipedPalleteRef->entryCount;
//    out.firstTag = (bipedTagReference*)getTagAddress(bipedPalleteRef->pointer);
//    return out;
//}



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



struct squadData
{
    MCCString squadName;
    uint16_t actorTypeIndex; // what we care about
    uint16_t platoonIndex;
    uint16_t initialState;
    uint16_t returnState;
    uint32_t flags;
    uint16_t uniqueLeaderType;
    char unknown[0xB4];
};
static_assert(sizeof(squadData) == 0xE8);

struct encounterData
{
    MCCString encounterName;
    uint32_t flags;
private:
    uint16_t mTeamIndex;
public:
    uint8_t unknown1;
    uint8_t unknown2;
    uint16_t searchBehaviour;
    uint16_t manualBSPIndex;
    rangef respawnDelay;
    char unknown3[0x48];
    uint16_t unknown4;
    uint16_t unknown5;
    tagBlock squads; // what we care about.. 0x80
    tagBlock platoons;
    tagBlock firingPositions;
    tagBlock playerStartingLocations;

    faction teamIndex() { return static_cast<faction>(mTeamIndex); }
};
static_assert(sizeof(encounterData) == 0xB0);

datum MapReader::MapReaderImpl::getEncounterSquadDatum(int encounterIndex, int squadIndex)
{
    // Note; needs to throw_from_hook since running from hook. No actually should try catch in enemy randomiser

    if (!scenarioAddress) throw CEERRuntimeException("scenarioAddress not loaded yet!");

    constexpr int encounterOffset = 0x42C;
    tagBlock* encounterBlock = (tagBlock*)(scenarioAddress + encounterOffset);

    if (encounterIndex > encounterBlock->entryCount) 
        throw CEERRuntimeException(std::format("recieved bad encounterIndex! encIndex {}, entryCount {}", encounterIndex, encounterBlock->entryCount));

    // is the squad datum possibly null too?
    auto thisEncounter = (encounterData*)(getTagAddress(encounterBlock->pointer) + (encounterIndex * sizeof(encounterData)));
    //PLOG_DEBUG << "thisEncounter: " << std::hex << thisEncounter;
    if (squadIndex > thisEncounter->squads.entryCount) throw CEERRuntimeException("recieved bad squadIndex!");

    auto thisSquad = (squadData*)(getTagAddress(thisEncounter->squads.pointer) + (squadIndex * sizeof(squadData)));
    //PLOG_DEBUG << "thisSquad: " << std::hex << thisSquad;
    //PLOG_DEBUG << "actorPalette: " << std::hex << getTagAddress(actorPalette->pointer);
    //PLOG_DEBUG << "thisSquad->actorTypeIndex" << thisSquad->actorTypeIndex;

    PLOG_DEBUG << "actorTypeIndex: " << thisSquad->actorTypeIndex;

    if (thisSquad->actorTypeIndex >= actorPalette->entryCount)  // sometimes bungie set the actor type to -1 to disable a squad.. very annoying
    {
        PLOG_ERROR << "actorType index >= actorPalette->entryCount, returning nullDatum";
        return nullDatum;
    }

    auto actorRef = (tagReference*)(getTagAddress(actorPalette->pointer) + (thisSquad->actorTypeIndex * 0x10));

    constexpr uint32_t actvMagic = stringToMagic("actv");
    if (actorRef->tagGroupMagic != actvMagic) throw CEERRuntimeException(std::format("failed to evaluate actv from squad, magic mistmatch! actual magic was: {} at {}", actorRef->tagGroupMagic, (uint64_t)actorRef));

    return actorRef->tagDatum;

}








tagBlock* MapReader::getActorPalette() { return impl.get()->getActorPalette(); }
//bipedPaletteWrapper MapReader::getBipedPalette() { return impl.get()->getBipedPalette(); }

std::string MapReader::getTagName(const datum& tagDatum) { return impl.get()->getTagName(tagDatum); }

//datum MapReader::getActorsBiped(const datum& actorDatum) { return impl.get()->getActorsBiped(actorDatum); }

faction MapReader::getBipedFaction(const datum& bipedDatum) { return impl.get()->getBipedFaction(bipedDatum); }

std::string MapReader::getObjectName(int nameIndex) { return impl.get()->getObjectName(nameIndex); }

std::span<tagElement> MapReader::getTagTable() { return impl.get()->getTagTable(); }

tagElement* MapReader::getTagElement(const datum& tagDatum) { return impl.get()->getTagElement(tagDatum); }

faction MapReader::getActorsFaction(const datum& actorDatum) { return impl.get()->getActorsFaction(actorDatum); }
datum MapReader::getEncounterSquadDatum(int encounterIndex, int squadIndex) { return impl.get()->getEncounterSquadDatum(encounterIndex, squadIndex); }
 
uintptr_t MapReader::getTagAddress(const datum& tagDatum) { return impl.get()->getTagAddress(tagDatum); }
uintptr_t MapReader::getTagAddress(uint32_t tagOffset) { return impl.get()->getTagAddress(tagOffset); }
void MapReader::cacheTagData(HaloLevel newLevel) { return impl.get()->cacheTagData(newLevel); }


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
