#pragma once

#include <cstdio>
#include <iostream>

#include <BWAPI.h>
#include <StormLib.h>

class SCXFileSection
{
public:
    SCXFileSection(const char* sectionName, int sectionLength) : name(sectionName), length(sectionLength)
    {
        data=new char[sectionLength];
    }

    ~SCXFileSection()
    {
        delete data;
    }

    std::string name;
    int length;
    char *data;
};

struct EntryUnit
{
    int32_t serial;
    int16_t x;
    int16_t y;
    int16_t id;
    int16_t relationType;
    int16_t special;
    int16_t flags;
    int8_t player;
    int8_t hitPoints;
    int8_t shieldPoints;
    int8_t energyPoints;
    int32_t resource;
    int16_t unitsInHangar;
    int16_t state;
    int32_t unused;
    int32_t relationUnit;
};

class UnitSectionInterpreter
{
public:
    UnitSectionInterpreter()
    {}

    UnitSectionInterpreter(SCXFileSection *section)
    {
        read(section);
    }

    bool read(SCXFileSection *section)
    {
        max_serial=1;

        if(!section)
            return false;

        char *data=section->data;
        int offset=0;

        while(offset<section->length)
        {
            EntryUnit *u=(EntryUnit*)&data[offset];

            if(u->serial>max_serial)max_serial=u->serial;

            offset+=sizeof(EntryUnit);

            EntryUnit *ucopy=new EntryUnit(*u);
            units.push_back(ucopy);
        }

        return true;
    }

    void outEntryUnit(EntryUnit *u)
    {
        std::cout
            <<"Serial:"<<u->serial
            <<" x:"<<u->x
            <<" y:"<<u->y
            <<" id:"<<u->id
            <<" relType:"<<u->relationType
            <<" special:"<<u->special
            <<" flags:"<<u->flags
            <<" player: "<<(int)u->player
            <<" hitPoints: "<<(int)u->hitPoints
            <<" shieldPoints: "<<(int)u->shieldPoints
            <<" energyPoints: "<<(int)u->energyPoints
            <<" resource:"<<u->resource
            <<" unitsInHangar:"<<u->unitsInHangar
            <<" state:"<<u->state
            <<" relUnit:"<<u->relationUnit
            <<"\n";
    }

    EntryUnit* addFromTemplate(EntryUnit *unit_template, int16_t x,int16_t y, int8_t player)
    {
        if(!unit_template)
        {
            std::cout<<"no template to add from!\n";
            return NULL;
        }
        EntryUnit *unew=new EntryUnit(*unit_template);
        unew->serial=++max_serial;
        unew->x=x;
        unew->y=y;
        unew->player=player;
        units.push_back(unew);
        return unew;
    }

    EntryUnit *findByID(int16_t id)
    {
        for(auto u : units)
        {
            if(u->id==id)
                return u;
        }
        std::cout<<"no unit found by id "<<id<<"\n";
        return 0;
    }

    void save(SCXFileSection *section)
    {
        delete section->data;
        section->length=units.size()*sizeof(EntryUnit);
        section->data=new char[section->length];
        EntryUnit *unit_ptr=(EntryUnit*)section->data;

        for(auto u : units)
        {
            *unit_ptr=*u;
            unit_ptr++;
        }
    }

    ~UnitSectionInterpreter()
    {
        for(auto u : units)
            delete u;
    }

    std::vector<EntryUnit *> units;
    int32_t max_serial;
};

class OwnerSectionInterpreter
{
public:

    enum OwnerType {Inactive,Computer_Game,Human_Player,Rescue_Passive,Unused,Computer,Human_Open,Neutral,Closed};
    char owners[12];

    OwnerSectionInterpreter()
    {}

    OwnerSectionInterpreter(SCXFileSection *section)
    {
        read(section);
    }

    bool read(SCXFileSection *section)
    {
        if(!section)
            return false;

        if(section->length==12)
        {
            for(int i=0;i<12;i++)
            {
                owners[i]=section->data[i];
            }
            return true;
        }
        else
            std::cout<<"OwnerSectionInterpreter: expected section length 12, but got: "<<section->length;

        return false;
    }

    void save(SCXFileSection *section)
    {
        delete section->data;
        section->length=12;
        section->data=new char[12];
        for(int i=0;i<12;i++)
        {
            section->data[i]=owners[i];
        }
    }

    void set(int playerID, OwnerType type)
    {
        if(playerID>=0 && playerID<12)
        {
            owners[playerID]=type;
        }
    }
};

class RaceSectionInterpreter
{
public:

    enum RaceType { Zerg,Terran,Protoss,Unused_Independent,Unused_Neutral,User_Selectable,Random,Inactive};
    char races[12];

    RaceSectionInterpreter()
    {}

    RaceSectionInterpreter(SCXFileSection *section)
    {
        read(section);
    }

    bool read(SCXFileSection *section)
    {
        if(!section)
            return false;

        if(section->length==12)
        {
            for(int i=0;i<12;i++)
                races[i]=section->data[i];
            return true;
        }
        else
            std::cout<<"RaceSectionInterpreter: expected section length 12, but got: "<<section->length;

        return false;
    }

    void save(SCXFileSection *section)
    {
        delete section->data;
        section->length=12;
        section->data=new char[12];
        for(int i=0;i<12;i++)
            section->data[i]=races[i];
    }

    void set(int playerID, RaceType type)
    {
        if(playerID>=0 && playerID<12)
        {
            races[playerID]=type;
        }
    }
};

class UpgradeSectionInterpreter
{
public:

    char max_upgrades[12][61];
    char start_upgrades[12][61];
    char global_max_upgrades[61];
    char global_start_upgrades[61];
    char use_global_defaults[12][61];

    UpgradeSectionInterpreter()
    {}

    UpgradeSectionInterpreter(SCXFileSection *section)
    {
        read(section);
    }

    bool read(SCXFileSection *section)
    {
        if(!section)
            return false;

        char *data=section->data;
        int num_upgrades;
        if(section->length==3*46*12+2*46)
            num_upgrades=46;    //classic SC
        else if(section->length==3*61*12+2*61)
            num_upgrades=61;    //Broodwar
        else
        {
            std::cout<<"UpgradeSectionInterpreter: unexpected section length: "<<section->length;
            return false;
        }

        for(int p=0;p<12;p++)
            for(int u=0;u<num_upgrades;u++)
                max_upgrades[p][u]=*(data++);
        for(int p=0;p<12;p++)
            for(int u=0;u<num_upgrades;u++)
                start_upgrades[p][u]=*(data++);
        for(int u=0;u<num_upgrades;u++)
            global_max_upgrades[u]=*(data++);
        for(int u=0;u<num_upgrades;u++)
            global_start_upgrades[u]=*(data++);
        for(int p=0;p<12;p++)
            for(int u=0;u<num_upgrades;u++)
                use_global_defaults[p][u]=*(data++);
        return true;
    }

    void save(SCXFileSection *section)
    {
        char *data=section->data;
        int num_upgrades;
        if(section->length==3*46*12+2*46)
            num_upgrades=46;    //classic SC
        else if(section->length==3*61*12+2*61)
            num_upgrades=61;    //Broodwar
        else
        {
            std::cout<<"UpgradeSectionInterpreter: unexpected section length: "<<section->length;
            return;
        }

        for(int p=0;p<12;p++)
            for(int u=0;u<num_upgrades;u++)
                *(data++)=max_upgrades[p][u];
        for(int p=0;p<12;p++)
            for(int u=0;u<num_upgrades;u++)
                *(data++)=start_upgrades[p][u];
        for(int u=0;u<num_upgrades;u++)
            *(data++)=global_max_upgrades[u];
        for(int u=0;u<num_upgrades;u++)
            *(data++)=global_start_upgrades[u];
        for(int p=0;p<12;p++)
            for(int u=0;u<num_upgrades;u++)
                *(data++)=use_global_defaults[p][u];

    }
        //playerID: 0-12 or -1 for global
        //start: current upgrade level or -1 to use global defaults
        //max: max upgrade level
    void set(int playerID,int upgradeID,int start, int max)
    {
        if(upgradeID<0||upgradeID>=61)
        {
            std::cout<<"UpgradeSectionInterpreter: invalid upgrade id "<<upgradeID<<"\n";
            return;
        }

        if(playerID>=0)
        {
            if(playerID<12)
            {
                if(start>=0)
                {
                    start_upgrades[playerID][upgradeID]=start;
                    max_upgrades[playerID][upgradeID]=max;
                    use_global_defaults[playerID][upgradeID]=0;
                }
                else
                    use_global_defaults[playerID][upgradeID]=1;
            }
        }
        else if(playerID==-1)
        {
            global_start_upgrades[upgradeID]=start;
            global_max_upgrades[upgradeID]=max;
        }
        else
            std::cout<<"UpgradeSectionInterpreter: invalid player id "<<playerID<<"\n";
    }
};

class TechSectionInterpreter
{
public:

    char available[12][44];
    char researched[12][44];
    char global_available[44];
    char global_researched[44];
    char use_global_defaults[12][44];

    TechSectionInterpreter()
    {}

    TechSectionInterpreter(SCXFileSection *section)
    {
        read(section);
    }

    bool read(SCXFileSection *section)
    {
        if(!section)
            return false;

        int num_techs;
        char *data=section->data;
        if(section->length==3*24*12+2*24)
            num_techs=24;
        else if(section->length==3*44*12+2*44)
            num_techs=44;
        else
        {
            std::cout<<"TechSectionInterpreter: unexpected section length: "<<section->length;
            return false;
        }

        for(int p=0;p<12;p++)
            for(int u=0;u<num_techs;u++)
                available[p][u]=*(data++);
        for(int p=0;p<12;p++)
            for(int u=0;u<num_techs;u++)
                researched[p][u]=*(data++);
        for(int u=0;u<num_techs;u++)
            global_available[u]=*(data++);
        for(int u=0;u<num_techs;u++)
            global_researched[u]=*(data++);
        for(int p=0;p<12;p++)
            for(int u=0;u<num_techs;u++)
                use_global_defaults[p][u]=*(data++);
        return true;
    }

    void save(SCXFileSection *section)
    {
        int num_techs;
        char *data=section->data;
        if(section->length==3*24*12+2*24)
            num_techs=24;
        else if(section->length==3*44*12+2*44)
            num_techs=44;
        else
        {
            std::cout<<"TechSectionInterpreter: unexpected section length: "<<section->length;
            return;
        }

        for(int p=0;p<12;p++)
            for(int u=0;u<num_techs;u++)
                *(data++)=available[p][u];
        for(int p=0;p<12;p++)
            for(int u=0;u<num_techs;u++)
                *(data++)=researched[p][u];
        for(int u=0;u<num_techs;u++)
            *(data++)=global_available[u];
        for(int u=0;u<num_techs;u++)
            *(data++)=global_researched[u];
        for(int p=0;p<12;p++)
            for(int u=0;u<num_techs;u++)
                *(data++)=use_global_defaults[p][u];
    }
        //playerID: 0-12 or -1 for global
        //available: 0/1 or -1 to use global defaults
        //researched: 0/1
    void set(int playerID,int techID,int _available, int _researched)
    {
        if(techID<0||techID>=44)
        {
            std::cout<<"TechSectionInterpreter: invalid tech id "<<techID<<"\n";
            return;
        }

        if(playerID>=0)
        {
            if(playerID<12)
            {
                if(_available>=0)
                {
                    available[playerID][techID]=_available;
                    researched[playerID][techID]=_researched;
                    use_global_defaults[playerID][techID]=0;
                }
                else
                    use_global_defaults[playerID][techID]=1;
            }
        }
        else if(playerID==-1)
        {
            global_available[techID]=_available;
            global_researched[techID]=_researched;
        }
        else
            std::cout<<"TechSectionInterpreter: invalid player id "<<playerID<<"\n";
    }
};


class UnitPropertiesSectionInterpreter
{
public:

    struct UnitPropertiesStruct
    {
        int8_t use_defaults[228];
        int32_t hitpoints[228]; //hitpoints*256
        int16_t shieldpoints[228];
        int8_t armorpoints[228];
        int16_t buildtime[228]; //1/60 second
        int16_t mineralcost[228];
        int16_t gascost[228];
        int16_t stringnumber[228];
        union
        {
            struct
            {
                int16_t weapondamage[100];
                int16_t weapondamage_upgrade_bonus[100];
            } classic;
            struct
            {
                int16_t weapondamage[130];
                int16_t weapondamage_upgrade_bonus[130];
            } broodwar;
        } weapons;
    } props;

    UnitPropertiesSectionInterpreter()
    {}

    UnitPropertiesSectionInterpreter(SCXFileSection *section)
    {
        read(section);
    }

    bool read(SCXFileSection *section)
    {
        if(!section)
            return false;

        UnitPropertiesStruct *data=(UnitPropertiesStruct*)section->data;
        if(section->length==sizeof(UnitPropertiesStruct))
        {
            props=*data;
        }
        else if(section->length==sizeof(UnitPropertiesStruct)-60*2)
        {
#define _COPY_PROPS(field) std::copy(std::begin(data->field), std::end(data->field), std::begin(props.field))
            _COPY_PROPS(armorpoints);
            _COPY_PROPS(use_defaults);
            _COPY_PROPS(hitpoints);
            _COPY_PROPS(shieldpoints);
            _COPY_PROPS(armorpoints);
            _COPY_PROPS(buildtime);
            _COPY_PROPS(mineralcost);
            _COPY_PROPS(gascost);
            _COPY_PROPS(stringnumber);
            std::copy(std::begin(data->weapons.classic.weapondamage), std::end(data->weapons.classic.weapondamage), std::begin(props.weapons.broodwar.weapondamage));
            std::copy(std::begin(data->weapons.classic.weapondamage_upgrade_bonus), std::end(data->weapons.classic.weapondamage_upgrade_bonus), std::begin(props.weapons.broodwar.weapondamage_upgrade_bonus));
#undef _COPY_PROPS
        }
        else
        {
            std::cout<<"UnitPropertiesSectionInterpreter: unexpected section length: "<<section->length<<" expected: "<<(sizeof(UnitPropertiesStruct)-60*2)<<" or "<<sizeof(UnitPropertiesStruct);
        }
        return false;
    }

    void save(SCXFileSection *section)
    {
        UnitPropertiesStruct *data=(UnitPropertiesStruct *)section->data;

        if(section->length==sizeof(UnitPropertiesStruct))
        {
            *data=props;
        }
        else if(section->length==sizeof(UnitPropertiesStruct)-60*2)
        {
#define _COPY_PROPS(field) std::copy(std::begin(props.field), std::end(props.field), std::begin(data->field))
            _COPY_PROPS(armorpoints);
            _COPY_PROPS(use_defaults);
            _COPY_PROPS(hitpoints);
            _COPY_PROPS(shieldpoints);
            _COPY_PROPS(armorpoints);
            _COPY_PROPS(buildtime);
            _COPY_PROPS(mineralcost);
            _COPY_PROPS(gascost);
            _COPY_PROPS(stringnumber);
            for(int i=0;i<100;i++)
            {
                data->weapons.classic.weapondamage[i]=props.weapons.broodwar.weapondamage[i];
                data->weapons.classic.weapondamage_upgrade_bonus[i]=props.weapons.broodwar.weapondamage_upgrade_bonus[i];
            }
#undef _COPY_PROPS
        }
        else
        {
            std::cout<<"UnitPropertiesSectionInterpreter: unexpected section length: "<<section->length;
        }
    }
};



class SCXFile
{
public:
    SCXFile(std::string filename)
    {
        read(filename);
    }
    ~SCXFile()
    {
        cleanup();
    }

    void cleanup()
    {
        for(auto s : sections)
            delete s;
    }

    void read(std::string filename)
    {
        HANDLE scxFile = NULL;
        HANDLE scenarioCHK = NULL;

        cleanup();

        if(SFileOpenArchive(filename.c_str(),0,STREAM_PROVIDER_FLAT|BASE_PROVIDER_FILE,&scxFile))
        {
            if(SFileOpenFileEx(scxFile, "staredit/scenario.chk", 0, &scenarioCHK))
            {
                DWORD dwBytes = 1;
                while(dwBytes > 0)
                {
                    char sectionName[5];
                    int sectionLength;

                    if(!SFileReadFile(scenarioCHK, sectionName, 4, &dwBytes, NULL))
                    {
                            //no more sections -> done.
                        break;
                    }
                    sectionName[4]=0;

                    if(!SFileReadFile(scenarioCHK, &sectionLength, 4, &dwBytes, NULL))
                    {
                        std::cout << "failed to read section length of "<<sectionName<<"\n";
                        break;
                    }

                    SCXFileSection* section=new SCXFileSection(sectionName,sectionLength);
                    if(!SFileReadFile(scenarioCHK, section->data, sectionLength, &dwBytes, NULL))
                    {
                        std::cout << "failed to read section data of "<<sectionName<<" ("<<sectionLength<<")\n";
                        break;
                    }
                    else dwBytes=1;
                    sections.push_back(section);
                }
                SFileCloseFile(scenarioCHK);
            }
            else
                std::cout<<"Failed to read scenario.chk in "<<filename<<"\n";

            SFileCloseArchive(scxFile);
        }
        else
            std::cout<<"Failed to open "<<filename<<" for reading\n";
    }

    int getTotalSize()
    {
        int ret=0;
        for(auto s : sections)
        {
            ret+=8+s->length;
        }
        return ret;
    }

    void write(std::string filename)
    {
        HANDLE scxFile = NULL;
        HANDLE scenarioCHK = NULL;
        DWORD dwWriteFlags = MPQ_COMPRESSION_PKWARE;

        std::remove(filename.c_str());

        if(SFileCreateArchive(filename.c_str(),MPQ_CREATE_LISTFILE,8,&scxFile))
        {
            if(SFileCreateFile(scxFile,"staredit/scenario.chk",0,getTotalSize(),0,MPQ_FILE_REPLACEEXISTING|MPQ_FILE_COMPRESS,&scenarioCHK))
            {
                for(auto s : sections)
                {
                    if(SFileWriteFile(scenarioCHK,s->name.c_str(),4,dwWriteFlags))
                    {
                        if(SFileWriteFile(scenarioCHK,&s->length,4,dwWriteFlags))
                        {
                            if(SFileWriteFile(scenarioCHK,s->data,s->length,dwWriteFlags))
                            {
                                //good :)
                            }
                            else
                                std::cout<<"Failed to write section data of "<<s->name<<" to scenario.chk in "<<filename<<"\n";
                        }
                        else
                            std::cout<<"Failed to write section length of "<<s->name<<" to scenario.chk in "<<filename<<"\n";
                    }
                    else
                        std::cout<<"Failed to write section "<<s->name<<" to scenario.chk in "<<filename<<"\n";
                }
                if(SFileFinishFile(scenarioCHK))
                {
                    //good
                }
                else
                    std::cout<<"Failed to finish write to scenario.chk in "<<filename<<"\n";
            }
            else
                std::cout<<"Failed to write scenario.chk in "<<filename<<"\n";

            if(SFileCloseArchive(scxFile))
            {
                //good
            }
            else
                std::cout<<"Failed to close "<<filename<<" after writing\n";
        }
        else
            std::cout<<"Failed to open "<<filename<<" for writing\n";
    }

    SCXFileSection *getSection(std::string name)
    {
        for(auto s : sections)
        {
            if(s->name == name)
                return s;
        }
        std::cout<<"section "<<name<<" not found!\n";
        return 0;
    }

protected:
    std::vector<SCXFileSection*> sections;
};

class BWMapModifier
{
public:
    BWMapModifier(std::string filename) :
        scxTemplate("bwapi-data/maps/template.scx"),
        unitTemplate(scxTemplate.getSection("UNIT")),
        scxMap(filename)
    {
        sectionUNIT=scxMap.getSection("UNIT");
        if(sectionUNIT)
            units.read(sectionUNIT);

        sectionIOWN=scxMap.getSection("IOWN");
        sectionOWNR=scxMap.getSection("OWNR");
        if(sectionIOWN)
            owners.read(sectionIOWN);
        if(sectionOWNR)
            owners.read(sectionOWNR);

        sectionSIDE=scxMap.getSection("SIDE");
        if(sectionSIDE)
            races.read(sectionSIDE);

        sectionUPGR=scxMap.getSection("UPGR");
        if(sectionUPGR)
            upgrades.read(sectionUPGR);
        sectionPUPx=scxMap.getSection("PUPx");
        if(sectionPUPx)
            upgrades.read(sectionPUPx);

        sectionPTEC=scxMap.getSection("PTEC");
        if(sectionPTEC)
            techs.read(sectionPTEC);
        sectionPTEx=scxMap.getSection("PTEx");
        if(sectionPTEx)
            techs.read(sectionPTEx);

        sectionUNIS=scxMap.getSection("UNIS");
        if(sectionUNIS)
            unitProps.read(sectionUNIS);
        sectionUNIx=scxMap.getSection("UNIx");
        if(sectionUNIx)
            unitProps.read(sectionUNIx);
    }

    void save(std::string filename)
    {
        scxMap.write(filename+".scx");

        if(sectionUNIT)
            units.save(sectionUNIT);

        if(sectionIOWN)
            owners.save(sectionIOWN);
        if(sectionOWNR)
            owners.save(sectionOWNR);
        
        if(sectionSIDE)
            races.save(sectionSIDE);

        if(sectionUPGR)
            upgrades.save(sectionUPGR);
        if(sectionPUPx)
            upgrades.save(sectionPUPx);

        if(sectionPTEC)
            techs.save(sectionPTEC);
        if(sectionPTEx)
            techs.save(sectionPTEx);

        if(sectionUNIS)
            unitProps.save(sectionUNIS);
        if(sectionUNIx)
            unitProps.save(sectionUNIx);

        scxMap.write(filename);
    }

    EntryUnit *addUnit(BWAPI::UnitType type, BWAPI::Position pos, int playerID)
    {
        return units.addFromTemplate(unitTemplate.findByID(type.getID()),(int16_t)pos.x(),(int16_t)pos.y(),(int8_t)playerID);
    }

    void setOwner(int playerID, OwnerSectionInterpreter::OwnerType type)
    {
        owners.set(playerID,type);
    }
    void setRace(int playerID, RaceSectionInterpreter::RaceType type)
    {
        races.set(playerID,type);
    }
        //playerID: -1 for global, start: -1 for use_global_defaults
    void setUpgrade(int playerID, const BWAPI::UpgradeType &ut, int start, int max)
    {
        upgrades.set(playerID,ut.getID(),start,max);
    }
        //playerID: -1 for global,  available: -1 for use_global_defaults
    void setTech(int playerID, const BWAPI::TechType &tt, int available, int researched)
    {
        techs.set(playerID,tt.getID(),available,researched);
    }


private:
    SCXFile scxTemplate;
    UnitSectionInterpreter unitTemplate;

    SCXFile scxMap;
    SCXFileSection *sectionUNIT;
    UnitSectionInterpreter units;

    SCXFileSection *sectionIOWN,*sectionOWNR;
    OwnerSectionInterpreter owners;

    SCXFileSection *sectionSIDE;
    RaceSectionInterpreter races;

    SCXFileSection *sectionUPGR,*sectionPUPx;
    UpgradeSectionInterpreter upgrades;

    SCXFileSection *sectionPTEC,*sectionPTEx;
    TechSectionInterpreter techs;

    SCXFileSection *sectionUNIS,*sectionUNIx;
    UnitPropertiesSectionInterpreter unitProps;
};