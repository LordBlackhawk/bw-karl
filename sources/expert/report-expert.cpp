/*
    ToDo: EXAMPLE OF IMPROVED GRAPH:
    
    { rank=min;
    n1 [label="Zerg Larva",shape=box,color=green]
    n2 [label="Zerg Hatchery",shape=box,color=green]
    n4 [label="Zerg Drone",shape=box,color=green]
    n5 [label="Zerg Drone",shape=box,color=green]
    n6 [label="Zerg Drone",shape=box,color=green]
    n7 [label="Zerg Overlord",shape=box,color=green]
    n8 [label="Zerg Larva",shape=box,color=green]
    n9 [label="Zerg Larva",shape=box,color=green]
    n10 [label="Resource Mineral Field",shape=box,color=blue]
    n11 [label="Resource Mineral Field",shape=box,color=blue]
    n12 [label="Resource Mineral Field",shape=box,color=blue]
    n14 [label="Resource Mineral Field",shape=box,color=blue]
    }
    subgraph cluster0 {
     { rank=min;
     n3 [label="Zerg Drone",shape=box,color=green]
     n13 [label="Resource Mineral Field",shape=box,color=blue]
     }
     n15 [label="GatherMinerals\nactive"]
     n3 -> n15 [color=black]
     n13 -> n15 [color=black]
     label="3x";
     color=black;
    }
*/

#include "report-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-ports.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "utils/options.hpp"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <windows.h>

REGISTER_EXPERT(ReportExpert)

namespace
{
    int reportInterval = 0;
}

DEF_OPTIONS
{
    po::options_description options("Report options");
    options.add_options()
            ("report",      po::value<int>(&reportInterval),         "Report interval in frames (0 = inactive).")
        ;
    return options;
}

bool ReportExpert::isApplicable(Blackboard* /*blackboard*/)
{
    return (reportInterval > 0);
}

namespace
{
    struct GraphWriter : public BasicVisitor
    {
        std::fstream                    stream;
        std::map<AbstractItem*, int>    itemNumbers;
        int                             counter;

        GraphWriter(const std::string& filename)
            : stream(filename, std::fstream::out), counter(1)
        { }

        ~GraphWriter()
        {
            stream.close();
        }

        void writeNode(AbstractItem* item, std::string label, std::string shape = "", std::string color = "")
        {
            int number = counter++;
            itemNumbers[item] = number;
            stream << "n" << number << " [label=\"" << label << "\"";
            if (shape != "")
                stream << ",shape=" << shape;
            if (color != "")
                stream << ",color=" << color;
            stream << "]\n";
        }

        void visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item)
        {
            writeNode(item, item->getUnitType().getName(), "box", "green");
        }

        void visitResourceBoundaryItem(ResourceBoundaryItem* item)
        {
            if (item->numberOfWorkers() > 0)
                writeNode(item, item->getUnitType().getName(), "box", "blue");
        }

        void visitEnemyUnitBoundaryItem(EnemyUnitBoundaryItem* item)
        {
            writeNode(item, item->getUnitType().getName(), "box", "red");
        }
        
        void writePlanItem(AbstractPlanItem* item, std::string name)
        {
            std::stringstream buffer;
            buffer << name << "\\n";
            if (item->isActive()) {
                buffer << "active";
            } else {
                buffer << item->estimatedStartTime;
            }
            writeNode(item, buffer.str());
        }
        
        void visitGatherMineralPlanItem(GatherMineralsPlanItem* item)
        {
            writePlanItem(item, "GatherMinerals");
        }

        void visitMorphUnitPlanItem(MorphUnitPlanItem* item)
        {
            writePlanItem(item, "Morph(" + item->getUnitType().getName() + ")");
        }

        void visitMoveToPositionPlanItem(MoveToPositionPlanItem* item)
        {
            writePlanItem(item, "MoveTo");
        }

        void visitBuildPlanItem(BuildPlanItem* item)
        {
            writePlanItem(item, "Build(" + item->getUnitType().getName() + ")");
        }

        void visitAttackUnitPlanItem(AttackUnitPlanItem* item)
        {
            writePlanItem(item, "Attack");
        }
        
        void writeEdge(AbstractItem* lhs, AbstractItem* rhs, std::string color, std::string style = "", std::string label = "")
        {
            stream << "n" << itemNumbers[lhs] << " -> " << "n" << itemNumbers[rhs] << " [";
            if (color != "")
                stream << "color=" << color;
            if (style != "")
                stream << ",style=" << style;
            if (label != "")
                stream << ",label=\"" << label << "\"";
            stream << "]\n";
        }
        
        void visitProvideUnitPort(ProvideUnitPort* port)
        {
            if (port->isConnected())
                writeEdge(port->getOwner(), port->getConnectedPort()->getOwner(), "black");
        }
        
        void visitProvideMineralFieldPort(ProvideMineralFieldPort* port)
        {
            if (port->isConnected())
                writeEdge(port->getOwner(), port->getConnectedPort()->getOwner(), "black");
        }
        
        void visitProvideEnemyUnitPort(ProvideEnemyUnitPort* port)
        {
            if (port->isConnected())
                writeEdge(port->getOwner(), port->getConnectedPort()->getOwner(), "black");
        }

        void writeEdges(AbstractItem* item)
        {
            for (auto it : item->ports)
                it->acceptVisitor(this);
        }

        void write(Blackboard* blackboard)
        {
            stream << "digraph G {\n";
            stream << "{ rank=same;\n";
            for (auto it : blackboard->getBoundaries())
                it.second->acceptVisitor(this);
            stream << "}\n";
            for (auto it : blackboard->getItems())
                it->acceptVisitor(this);
            for (auto it : blackboard->getBoundaries())
                writeEdges(it.second);
            for (auto it : blackboard->getItems())
                writeEdges(it);
            stream << "}";
        }
    };

    struct ReportWriter
    {
        std::string prefix;
        std::fstream stream;

        ReportWriter(const std::string& p)
            : prefix(p), stream(prefix + ".html", std::fstream::out | std::fstream::app)
        { }

        ~ReportWriter()
        {
            stream.close();
        }

        void writeOpening()
        {
            stream << "<html><body>";
        }

        void writeFrame(Blackboard* blackboard)
        {
            int time = blackboard->getLastUpdateTime() / 24;
            stream << std::setfill('0')
                << "<h1>"
                << std::setw(2) << time/3600 << ":"
                << std::setw(2) << (time/60)%60 << ":"
                << std::setw(2) << time%60
                << " (Frame " << blackboard->getLastUpdateTime() << ")</h1>";

            std::stringstream builder;
            builder << prefix << "/frame" << blackboard->getLastUpdateTime();
            std::string filename = builder.str();
            GraphWriter writer(filename + ".gv");
            writer.write(blackboard);

            stream << "<img src=\"../" << filename << ".png\" alt=\"\">";
        }

        void writeEnding()
        {
            stream << "</body></html>";
        }
    };
}

ReportExpert::ReportExpert()
    : nextReportTime(3)
{
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    std::stringstream stream;
    stream << std::setfill('0')
        << "./logs/report-"
        << std::setw(4) << (timeinfo->tm_year + 1900) << "-"
        << std::setw(2) << (timeinfo->tm_mon + 1) << "-"
        << std::setw(2) << timeinfo->tm_mday << "-"
        << std::setw(2) << timeinfo->tm_hour << "-"
        << std::setw(2) << timeinfo->tm_min << "-"
        << std::setw(2) << timeinfo->tm_sec;
    prefix = stream.str();

    CreateDirectory(prefix.c_str(), NULL);
    ReportWriter writer(prefix);
    writer.writeOpening();
}

ReportExpert::~ReportExpert()
{
    ReportWriter writer(prefix);
    writer.writeEnding();

    system("make -C ./logs/");
}

bool ReportExpert::tick(Blackboard* blackboard)
{
    if (blackboard->getLastUpdateTime() < nextReportTime)
        return true;

    ReportWriter writer(prefix);
    writer.writeFrame(blackboard);
    nextReportTime = blackboard->getLastUpdateTime() + reportInterval;
    return true;
}
