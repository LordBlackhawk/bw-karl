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

    void addFromTemplate(EntryUnit *unit_template, int16_t x,int16_t y, int8_t player)
    {
        if(!unit_template)
        {
            std::cout<<"no template to add from!\n";
            return;
        }
        EntryUnit *unew=new EntryUnit(*unit_template);
        unew->serial=++max_serial;
        unew->x=x;
        unew->y=y;
        unew->player=player;
        units.push_back(unew);
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
        sectionUnit=scxMap.getSection("UNIT");
        if(sectionUnit)
            units.read(sectionUnit);
    }

    void save(std::string filename)
    {
        if(sectionUnit)
            units.save(sectionUnit);
        scxMap.write(filename);
    }

    void addUnit(BWAPI::UnitType type, BWAPI::Position pos, int playerID)
    {
        units.addFromTemplate(unitTemplate.findByID(type.getID()),(int16_t)pos.x(),(int16_t)pos.y(),(int8_t)playerID);
    }

private:
    SCXFile scxTemplate;
    UnitSectionInterpreter unitTemplate;

    SCXFile scxMap;
    SCXFileSection *sectionUnit;
    UnitSectionInterpreter units;
};