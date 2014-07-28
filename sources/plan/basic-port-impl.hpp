#pragma once

#include "plan-item.hpp"

template <class DerivedClass, class ConnectionClass, bool Require>
class BasicPortImpl : public AbstractPort
{
    public:
        typedef BasicPortImpl<DerivedClass, ConnectionClass, Require> BaseClass;

        BasicPortImpl(AbstractItem* o)
            : AbstractPort(o), connection(NULL)
        { }
        
        ~BasicPortImpl()
        {
            This()->disconnect();
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
            disconnect();
            if (port != NULL) {
                if (Require) {
                    connection = port;
                    port->connection = This();
                } else {
                    port->connectTo(This());
                }
            }
        }

        void disconnect()
        {
            if (connection != NULL) {
                if (Require) {
                    connection->connection = NULL;
                    connection = NULL;
                } else {
                    connection->disconnect();
                }
            }
        }
        
        inline bool isConnected() const
        {
            return connection != NULL;
        }

    protected:
        friend class BasicPortImpl<ConnectionClass, DerivedClass, !Require>;
        ConnectionClass* connection;

        DerivedClass* This()
        {
            return static_cast<DerivedClass*>(this);
        }
};