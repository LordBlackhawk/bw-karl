#pragma once

#include "plan-item.hpp"
#include <iostream>

template <class DerivedClass, class ConnectionClass, bool Require, bool FreeOnDisconnect>
class BasicPortImpl : public AbstractPort
{
    public:
        typedef BasicPortImpl<DerivedClass, ConnectionClass, Require, FreeOnDisconnect> BaseClass;

        int estimatedDuration;

        BasicPortImpl(AbstractItem* o)
            : AbstractPort(o), estimatedDuration(1), connection(NULL), disconnectLevel(0)
        { }

        ~BasicPortImpl()
        {
            ++disconnectLevel;
            This()->staticDisconnect();
        }

        bool isRequirePort() const override
        {
            return Require;
        }

        bool isActiveConnection() const override
        {
            return isConnected() && isActive() && connection->isActive();
        }

        void updateEstimates() override
        {
            if (Require) {
                estimatedTime = (isConnected()) ? connection->estimatedTime : INFINITE_TIME;
            } else {
                // Remark this method is only called if owner is of type AbstractPlanItem!!!
                estimatedTime = static_cast<AbstractPlanItem*>(owner)->estimatedStartTime + std::max(1, estimatedDuration);
            }
        }

        void connectTo(ConnectionClass* port)
        {
            //assert(!isActiveConnection() && "Active connection can not be changed!");
            if (connection == port)
                return;
            staticDisconnect();
            if (port != NULL) {
                if (Require) {
                    port->staticDisconnect();
                    connection = port;
                    port->connection = This();
                } else {
                    port->connectTo(This());
                }
            }
        }

        void staticDisconnect()
        {
            if (isConnected()) {
                if (Require) {
                    connection->connection = NULL;
                    connection->disconnect();
                    connection = NULL;
                } else {
                    connection->disconnect();
                }
            }
        }

        void disconnect() override
        {
            ++disconnectLevel;
            staticDisconnect();
            --disconnectLevel;
            if (FreeOnDisconnect && (disconnectLevel == 0))
                delete this;
        }

        inline bool isConnected() const
        {
            return connection != NULL;
        }

        inline ConnectionClass* getConnectedPort() const
        {
            return connection;
        }

    protected:
        friend class BasicPortImpl<ConnectionClass, DerivedClass, !Require, false>;
        friend class BasicPortImpl<ConnectionClass, DerivedClass, !Require, true>;
        ConnectionClass* connection;

        DerivedClass* This()
        {
            return static_cast<DerivedClass*>(this);
        }

    private:
        int disconnectLevel;
};