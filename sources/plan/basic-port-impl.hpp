#pragma once

#include "plan-item.hpp"
#include <iostream>

template <class DerivedClass, class ConnectionClass, bool Require, bool FreeOnDisconnect>
class BasicPortImpl : public AbstractPort
{
    public:
        typedef BasicPortImpl<DerivedClass, ConnectionClass, Require, FreeOnDisconnect> BaseClass;

        BasicPortImpl(AbstractItem* o)
            : AbstractPort(o), connection(NULL)
        { }

        ~BasicPortImpl()
        {
            This()->staticDisconnect();
        }

        bool isRequirePort() const override
        {
            return Require;
        }

        bool isActiveConnection() const override
        {
            return isActive() && isConnected() && connection->isActive();
        }

        void updateEstimates()
        {
            if (Require)
                estimatedTime = (isConnected()) ? connection->estimatedTime : INFINITE_TIME;
        }

        void connectTo(ConnectionClass* port)
        {
            staticDisconnect();
            if (port != NULL) {
                if (Require) {
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
                    connection = NULL;
                } else {
                    connection->disconnect();
                }
            }
        }

        void disconnect() override
        {
            if (connection != NULL) {
                staticDisconnect();
                if (FreeOnDisconnect)
                    delete this;
            }
        }

        inline bool isConnected() const
        {
            return connection != NULL;
        }

    protected:
        friend class BasicPortImpl<ConnectionClass, DerivedClass, !Require, false>;
        friend class BasicPortImpl<ConnectionClass, DerivedClass, !Require, true>;
        ConnectionClass* connection;

        DerivedClass* This()
        {
            return static_cast<DerivedClass*>(this);
        }
};