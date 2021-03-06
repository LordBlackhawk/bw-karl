// ToDo:
//  *

#include "resources.hpp"
#include "mineral-line.hpp"
#include "arena.hpp"
#include "container-helper.hpp"
#include "valuing.hpp"
#include "log.hpp"
#include "circle-buffer.hpp"
#include <algorithm>

using namespace BWAPI;

namespace
{
    struct ResourcesPreconditionInternal;
    
    std::vector<ResourcesPreconditionInternal*> reslist;
    int indexcounter = 0;

    struct ResourcesPreconditionInternal : public ResourcesPrecondition
    {
        int index;
        
        ResourcesPreconditionInternal(int m, int g)
            : ResourcesPrecondition(Precondition::Impossible, m, g)
        {
            reslist.push_back(this);
            index = ++indexcounter;
        }

        virtual ~ResourcesPreconditionInternal()
        {
            Containers::remove(reslist, this);
        }
        
        ctype sortValue() const
        {
            return valueResources(time, wishtime, index);
        }
    };
    
    struct ResourcesSorter
    {
        bool operator () (ResourcesPreconditionInternal* lhs, ResourcesPreconditionInternal* rhs) const
        {
            return lhs->sortValue() < rhs->sortValue();
        }
    };
}

ResourcesPrecondition* getResources(int m, int g)
{
    return new ResourcesPreconditionInternal(m, g);
}

ResourcesPrecondition* getResources(const BWAPI::UnitType& ut)
{
    return getResources(ut.mineralPrice(), ut.gasPrice());
}

ResourcesPrecondition* getResources(const BWAPI::TechType& tt)
{
    return getResources(tt.mineralPrice(), tt.gasPrice());
}

ResourcesPrecondition* getResources(const BWAPI::UpgradeType& gt)
{
    return getResources(gt.mineralPrice(), gt.gasPrice());
}

namespace
{
    bool bGasOnDemand       = false;
    bool bGasOnDemandActive = false;
    int lastChange          = 0;
    
    void checkGasOnDemand()
    {
        if (!bGasOnDemand)
            return;

        int cur    = Broodwar->self()->gas();
        int needed = 0;
        for (auto it : reslist)
            needed += it->gas;

        bool needGas = needed > cur;
        if (needGas != bGasOnDemandActive)
            if (lastChange + 10 < Broodwar->getFrameCount())
        {
            //LOG << "change gas status to " << (needGas ? "" : "not ") << "mining.";
            setGasWorkerPerRefinery(needGas ? 3 : 0);
            bGasOnDemandActive = needGas;
            lastChange = Broodwar->getFrameCount();
        }
    }
}

void setGasOnDemand(bool value)
{
    bGasOnDemand       = value;
    bGasOnDemandActive = false;
    setGasWorkerPerRefinery(0);
}

void ResourcesCode::onMatchBegin()
{
    indexcounter       = 0;
    bGasOnDemand       = false;
    bGasOnDemandActive = false;
}

void ResourcesCode::onMatchEnd()
{
    reslist.clear();
}

void ResourcesCode::onTick()
{
    checkGasOnDemand();

    std::stable_sort(reslist.begin(), reslist.end(), ResourcesSorter());

    Player* self = Broodwar->self();
    int cur_m = self->minerals();
    int cur_g = self->gas();
    
    auto it    = reslist.begin();
    auto itend = reslist.end();
    while ((it != itend) && ((*it)->minerals <= cur_m) && ((*it)->gas <= cur_g)) {
        (*it)->time = 0;
        cur_m -= (*it)->minerals;
        cur_g -= (*it)->gas;
        ++it;
    }
    auto itbegin = it;
    
    for (it=itbegin; it!=itend; ++it)
        (*it)->time = 1;
    
    int rest_m = cur_m;
    it = itbegin;
    while ((it != itend) && (rest_m >= 0)) {
        if ((*it)->minerals <= rest_m && (*it)->gas == 0) {
            (*it)->time = 0;
            cur_m -= (*it)->minerals;
        }
        rest_m -= (*it)->minerals;
        ++it;
    }
    
    auto pit    = estimatedProduction.begin();
    auto pitend = estimatedProduction.end();
    int prod_m  = pit->minerals;
    int prod_g  = pit->gas;
    int time    = Broodwar->getFrameCount();
    ++pit;
    cur_m *= MineralFactor;
    cur_g *= MineralFactor;
    
    for (it=itbegin; it!=itend; ++it) {
        if ((*it)->time == 0) continue;
        int esttime;
        bool cont = true;
        while (cont) {
            int esttime_m = time + std::max(0, (MineralFactor * (*it)->minerals - cur_m) / prod_m);
            int esttime_g = time + std::max(0, (MineralFactor * (*it)->gas      - cur_g) / prod_g);
            esttime = std::max(esttime_m, esttime_g);
            
            cont = (pit != pitend) && (esttime > pit->time);
            if (cont) {
                int dt = pit->time - time;
                cur_m += dt * prod_m;
                cur_g += dt * prod_g;
                time  += dt;
                prod_m = pit->minerals;
                prod_g = pit->gas;
                ++pit;
            }
        }
        
        int dt = esttime - time;
        (*it)->time = esttime;
        cur_m += dt * prod_m - MineralFactor * (*it)->minerals;
        cur_g += dt * prod_g - MineralFactor * (*it)->gas;
        time  += dt;
    }
}

namespace
{
    CircleBuffer<int, 1000> mineralCirc(50);
    CircleBuffer<int, 1000> gasCirc(0);
}

int estimateResourcesAt(int time)
{
    int result = Broodwar->self()->minerals();
    Production& prod = estimatedProduction[0];
    result += ((time - Broodwar->getFrameCount()) * prod.minerals) / 1000;
    for (auto it : reslist)
        result -= it->minerals;
    return result;
}

void ResourcesCode::onDrawPlan(HUDTextOutput& /*hud*/)
{
    if (isArena)
        return;

    Player* self = Broodwar->self();
    int gat_m = self->gatheredMinerals();
    int gat_g = self->gatheredGas();
    int prod_m = gat_m - mineralCirc.get();
    int prod_g = gat_g - gasCirc.get();
    mineralCirc.add(gat_m);
    gasCirc.add(gat_g);
    
    Production& prod = estimatedProduction[0];
    
    Broodwar->drawTextScreen(440, 16, "\x07%d (%d)", prod.minerals, prod_m);
    Broodwar->drawTextScreen(520, 16, "\x07%d (%d)", prod.gas,      prod_g);
    
    int active_res   = 0;
    int res_minerals = 0;
    int res_gas      = 0;
    for (auto it : reslist) {
        if (it->time == 0)
            ++active_res;
        res_minerals += it->minerals;
        res_gas      += it->gas;
    }
    
    Broodwar->drawTextScreen(440, 32, "\x08%d", res_minerals);
    Broodwar->drawTextScreen(520, 32, "\x08%d", res_gas);
    Broodwar->drawTextScreen(480, 40, "\x07%d/\x08%d", active_res, reslist.size());

    /*
    int yvalue = 16;
    for (auto it : reslist) {
        if (it->time == 0) {
            Broodwar->drawTextScreen(460, yvalue, "\x07%d", it->minerals);
            Broodwar->drawTextScreen(520, yvalue, "\x07%d", it->gas);
            Broodwar->drawTextScreen(560, yvalue, "\x07--> %d", it->wishtime);
            yvalue += 16;
        } else {
            Broodwar->drawTextScreen(460, yvalue, "\x08%d", it->minerals);
            Broodwar->drawTextScreen(520, yvalue, "\x08%d", it->gas);
            Broodwar->drawTextScreen(560, yvalue, "\x08--> %d", it->wishtime);
            yvalue += 16;
            //break;
        }
    }
    */
}
