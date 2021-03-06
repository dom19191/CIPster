/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 *
 ******************************************************************************/

#include <vector>
#include <string.h>

#include "appcontype.h"
#include "cipconnectionmanager.h"

struct ExclusiveOwnerConnection
{
    int output_assembly;        ///< the O-to-T point for the connection
    int input_assembly;         ///< the T-to-O point for the connection
    int config_assembly;        ///< the config point for the connection
    CipConn connection_data;    ///< the connection data, only one connection is allowed per O-to-T point

    ExclusiveOwnerConnection( int aOutputAssembly=0, int aInputAssembly=0, int aConfigAssembly=0 ) :
        output_assembly( aOutputAssembly ),
        input_assembly( aInputAssembly ),
        config_assembly( aConfigAssembly )
    {}
};


struct InputOnlyConnection
{
    int output_assembly;        ///< the O-to-T point for the connection
    int input_assembly;         ///< the T-to-O point for the connection
    int config_assembly;        ///< the config point for the connection
    CipConn connection_data[CIPSTER_CIP_NUM_INPUT_ONLY_CONNS_PER_CON_PATH]; ///< the connection data

    InputOnlyConnection( int aOutputAssembly = 0, int aInputAssembly=0, int aConfigAssembly=0 ) :
        output_assembly( aOutputAssembly ),
        input_assembly( aInputAssembly ),
        config_assembly( aConfigAssembly )
    {}
};


struct ListenOnlyConnection
{
    int output_assembly;        ///< the O-to-T point for the connection
    int input_assembly;         ///< the T-to-O point for the connection
    int config_assembly;        ///< the config point for the connection
    CipConn connection_data[CIPSTER_CIP_NUM_LISTEN_ONLY_CONNS_PER_CON_PATH];    ///< the connection data

    ListenOnlyConnection( int aOutputAssembly=0, int aInputAssembly=0, int aConfigAssembly=0 ) :
        output_assembly( aOutputAssembly ),
        input_assembly( aInputAssembly ),
        config_assembly( aConfigAssembly )
    {}
};


static std::vector<ExclusiveOwnerConnection>    g_exclusive_owner;
static std::vector<InputOnlyConnection>         g_input_only;
static std::vector<ListenOnlyConnection>        g_listen_only;


static CipConn* getExclusiveOwnerConnection( CipConn* aConn, ConnectionManagerStatusCode* extended_error )
{
    CipConn* exclusive_owner_connection = NULL;

    for( unsigned i = 0; i < g_exclusive_owner.size();  ++i )
    {
        if( g_exclusive_owner[i].output_assembly == aConn->conn_path.consuming_path.GetInstanceOrConnPt()
         && g_exclusive_owner[i].input_assembly  == aConn->conn_path.producing_path.GetInstanceOrConnPt()
         && ( g_exclusive_owner[i].config_assembly == aConn->conn_path.config_path.GetInstanceOrConnPt() ||
            ( g_exclusive_owner[i].config_assembly == -1 && !aConn->conn_path.config_path.HasAny() ) )
          )
        {
            // check if on other connection point with the same output assembly is currently connected
            if( GetConnectedOutputAssembly( aConn->conn_path.consuming_path.GetInstanceOrConnPt() ) )
            {
                *extended_error = kConnectionManagerStatusCodeErrorOwnershipConflict;
                break;
            }

            exclusive_owner_connection = &g_exclusive_owner[i].connection_data;
            break;
        }
    }

    return exclusive_owner_connection;
}


static CipConn* getInputOnlyConnection( CipConn* aConn, ConnectionManagerStatusCode* extended_error )
{
    CipConn* input_only_connection = NULL;

    for( unsigned i = 0; i < g_input_only.size();  ++i )
    {
        // we have the same output assembly?
        if( g_input_only[i].output_assembly == aConn->conn_path.consuming_path.GetInstanceOrConnPt() )
        {
            if( g_input_only[i].input_assembly != aConn->conn_path.producing_path.GetInstanceOrConnPt() )
            {
                *extended_error = kConnectionManagerStatusCodeInvalidProducingApplicationPath;
                break;
            }

            if( g_input_only[i].config_assembly != aConn->conn_path.config_path.GetInstanceOrConnPt() )
            {
                *extended_error = kConnectionManagerStatusCodeInconsistentApplicationPathCombo;
                break;
            }

            for( int j = 0; j < CIPSTER_CIP_NUM_LISTEN_ONLY_CONNS_PER_CON_PATH; j++ )
            {
                if( kConnectionStateNonExistent == g_input_only[i].connection_data[j].state )
                {
                    return &g_input_only[i].connection_data[j];
                }
            }

            *extended_error = kConnectionManagerStatusCodeTargetObjectOutOfConnections;
            break;
        }
    }

    return input_only_connection;
}


static CipConn* getListenOnlyConnection( CipConn* aConn, ConnectionManagerStatusCode* extended_error )
{
    CipConn* listen_only_connection = NULL;

    if( aConn->t_to_o_ncp.ConnectionType() != kIOConnTypeMulticast )
    {
        // a listen only connection has to be a multicast connection.
        *extended_error = kConnectionManagerStatusCodeNonListenOnlyConnectionNotOpened;

        // maybe not the best error message however there is no suitable definition in the cip spec
        return NULL;
    }

    for( unsigned i = 0; i < g_listen_only.size();  ++i )
    {
        // we have the same output assembly?
        if( g_listen_only[i].output_assembly == aConn->conn_path.consuming_path.GetInstanceOrConnPt() )
        {
            if( g_listen_only[i].input_assembly != aConn->conn_path.producing_path.GetInstanceOrConnPt() )
            {
                *extended_error = kConnectionManagerStatusCodeInvalidProducingApplicationPath;
                break;
            }

            if( g_listen_only[i].config_assembly != aConn->conn_path.config_path.GetInstanceOrConnPt() )
            {
                *extended_error = kConnectionManagerStatusCodeInconsistentApplicationPathCombo;
                break;
            }

            if( NULL == GetExistingProducerMulticastConnection( aConn->conn_path.producing_path.GetInstanceOrConnPt() ) )
            {
                *extended_error = kConnectionManagerStatusCodeNonListenOnlyConnectionNotOpened;
                break;
            }

            for( int j = 0; j < CIPSTER_CIP_NUM_LISTEN_ONLY_CONNS_PER_CON_PATH; j++ )
            {
                if( kConnectionStateNonExistent == g_listen_only[i].connection_data[j].state )
                {
                    return &g_listen_only[i].connection_data[j];
                }
            }

            *extended_error = kConnectionManagerStatusCodeTargetObjectOutOfConnections;
            break;
        }
    }

    return listen_only_connection;
}


bool ConfigureExclusiveOwnerConnectionPoint(
        int output_assembly,
        int input_assembly,
        int config_assembly )
{
    if( g_exclusive_owner.size() < CIPSTER_CIP_NUM_EXCLUSIVE_OWNER_CONNS )
    {
        g_exclusive_owner.push_back(
            ExclusiveOwnerConnection( output_assembly, input_assembly, config_assembly ) );
        return true;
    }

    return false;
}


bool ConfigureInputOnlyConnectionPoint(
        int output_assembly,
        int input_assembly,
        int config_assembly )
{
    if( g_input_only.size() < CIPSTER_CIP_NUM_INPUT_ONLY_CONNS )
    {
        g_input_only.push_back(
                InputOnlyConnection( output_assembly, input_assembly, config_assembly ) );
        return true;
    }

    return false;
}


bool ConfigureListenOnlyConnectionPoint(
        int output_assembly,
        int input_assembly,
        int config_assembly )
{
    if( g_listen_only.size() < CIPSTER_CIP_NUM_LISTEN_ONLY_CONNS )
    {
        g_listen_only.push_back(
            ListenOnlyConnection( output_assembly, input_assembly, config_assembly ) );
        return true;
    }

    return false;
}


CipConn* GetIoConnectionForConnectionData( CipConn* aConn,  ConnectionManagerStatusCode* extended_error )
{
    *extended_error = kConnectionManagerStatusCodeSuccess;

    CipConn* io_connection = getExclusiveOwnerConnection( aConn, extended_error );

    if( !io_connection )
    {
        if( kConnectionManagerStatusCodeSuccess == *extended_error )
        {
            // we found no connection and don't have an error so try input only next
            io_connection = getInputOnlyConnection( aConn, extended_error );

            if( !io_connection )
            {
                if( 0 == *extended_error )
                {
                    // we found no connection and don't have an error so try listen only next
                    io_connection = getListenOnlyConnection( aConn, extended_error );

                    if( !io_connection &&  0 == *extended_error )
                    {
                        // no application connection type was found that suits the given data
                        // TODO check error code VS
                        *extended_error = kConnectionManagerStatusCodeInconsistentApplicationPathCombo;
                    }
                    else
                    {
                        aConn->instance_type = kConnInstanceTypeIoListenOnly;
                    }
                }
            }
            else
            {
                aConn->instance_type = kConnInstanceTypeIoInputOnly;
            }
        }
    }
    else
    {
        aConn->instance_type = kConnInstanceTypeIoExclusiveOwner;
    }

    if( io_connection )
    {
        CopyConnectionData( io_connection, aConn );
    }

    return io_connection;
}


CipConn* GetExistingProducerMulticastConnection( EipUint32 input_point )
{
    CipConn* producer_multicast_connection = g_active_connection_list;

    while( producer_multicast_connection )
    {
        if( producer_multicast_connection->instance_type == kConnInstanceTypeIoExclusiveOwner
         || producer_multicast_connection->instance_type == kConnInstanceTypeIoInputOnly )
        {
            if( input_point == producer_multicast_connection->conn_path.producing_path.GetInstanceOrConnPt()
                && producer_multicast_connection->t_to_o_ncp.ConnectionType() == kIOConnTypeMulticast
                && kEipInvalidSocket != producer_multicast_connection->producing_socket )
            {
                // we have a connection that produces the same input assembly,
                // is a multicast producer and manages the connection.
                break;
            }
        }

        producer_multicast_connection = producer_multicast_connection->next;
    }

    return producer_multicast_connection;
}


CipConn* GetNextNonControlMasterConnection( EipUint32 input_point )
{
    CipConn* next_non_control_master_connection = g_active_connection_list;

    while( next_non_control_master_connection )
    {
        if( next_non_control_master_connection->instance_type == kConnInstanceTypeIoExclusiveOwner
         || next_non_control_master_connection->instance_type == kConnInstanceTypeIoInputOnly )
        {
            if( input_point == next_non_control_master_connection->conn_path.producing_path.GetInstanceOrConnPt()
             && next_non_control_master_connection->t_to_o_ncp.ConnectionType() == kIOConnTypeMulticast
             && next_non_control_master_connection->producing_socket == kEipInvalidSocket )
            {
                // we have a connection that produces the same input assembly,
                // is a multicast producer and does not manages the connection.
                break;
            }
        }

        next_non_control_master_connection = next_non_control_master_connection
                                             ->next;
    }

    return next_non_control_master_connection;
}


void CloseAllConnectionsForInputWithSameType( EipUint32 input_point,  ConnInstanceType instance_type )
{
    CipConn* connection = g_active_connection_list;
    CipConn* connection_to_delete;

    while( connection )
    {
        if( instance_type == connection->instance_type &&
            input_point   == connection->conn_path.producing_path.GetInstanceOrConnPt() )
        {
            connection_to_delete = connection;
            connection = connection->next;

            CheckIoConnectionEvent(
                    connection_to_delete->conn_path.consuming_path.GetInstanceOrConnPt(),
                    connection_to_delete->conn_path.producing_path.GetInstanceOrConnPt(),
                    kIoConnectionEventClosed );

            // FIXME check if this is ok
            connection_to_delete->connection_close_function( connection_to_delete );
            // closeConnection(pstToDelete); will remove the connection from the active connection list
        }
        else
        {
            connection = connection->next;
        }
    }
}


void CloseAllConnections()
{
    CipConn* connection = g_active_connection_list;

    while( connection )
    {
        // FIXME check if m_pfCloseFunc would be suitable
        CloseConnection( connection );

        // Close connection will remove the connection from the list therefore we
        // need to get again the start until there is no connection left
        connection = g_active_connection_list;
    }
}


bool ConnectionWithSameConfigPointExists( EipUint32 config_point )
{
    CipConn* connection = g_active_connection_list;

    while( connection )
    {
        if( config_point == connection->conn_path.config_path.GetInstanceOrConnPt() )
        {
            break;
        }

        connection = connection->next;
    }

    return NULL != connection;
}


void InitializeIoConnectionData()
{
    // now done by "static C++ construction"
}


void DestroyIoConnectionData()
{
    g_exclusive_owner.clear();

    g_input_only.clear();

    g_listen_only.clear();
}
