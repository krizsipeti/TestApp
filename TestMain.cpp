//----------------------------------------------------------------------------
// N O L D U S   I N F O R M A T I O N   T E C H N O L O G Y   B . V .
//----------------------------------------------------------------------------
// Project    :   TestApp
// Module     :   TestApp
// File       :   TestMain.cpp
//----------------------------------------------------------------------------
// Revisions  :
// 09-01-2020 Krizsán Péter             - Original version
// https://blog.3d-logic.com/2015/05/20/signalr-native-client/
//----------------------------------------------------------------------------
#include <iostream>
#include <sstream>
#include "hub_connection_builder.h"
#include "log_writer.h"
#include <future>
#include "signalr_value.h"

using namespace signalr;

#pragma comment(lib, "..\\build.release\\bin\\Debug\\signalrclient.lib")


class logger : public signalr::log_writer
{
    // Inherited via log_writer
    virtual void __cdecl write(const std::string & entry) override
    {
        std::cout << entry << std::endl;
    }
};


void send_message(signalr::hub_connection& connection, const std::string& message)
{
    std::vector<signalr::value> arr{ std::string("c++"), message };
    signalr::value args(arr);

    // if you get an internal compiler error uncomment the lambda below or install VS Update 4
    connection.invoke("Send", args, [](const signalr::value& value, std::exception_ptr exception)
    {
        try
        {
            if (exception)
            {
                std::rethrow_exception(exception);
            }

            if (value.is_string())
            {
                std::cout << "Received: " << value.as_string() << std::endl;
            }
            else
            {
                std::cout << "hub method invocation has completed" << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::cout << "Error while sending data: " << e.what() << std::endl;
        }
    });
}


int main()
{
    // Create the hub connection
    hub_connection connection = hub_connection_builder::create("http://noldushu.dlinkddns.com:5000/webRtcHub")
        .with_logging(std::make_shared<logger>(), signalr::trace_level::all)
        .build();

    connection.on("ReceiveMessage", [](const signalr::value & m)
    {
        std::cout << std::endl << m.as_array()[0].as_string() << std::endl;
    });

    std::promise<void> task;
    connection.start([&connection, &task](std::exception_ptr exception)
    {
        if (exception)
        {
            try
            {
                std::rethrow_exception(exception);
            }
            catch (const std::exception & ex)
            {
                std::cout << "exception when starting connection: " << ex.what() << std::endl;
            }
            task.set_value();
            return;
        }

        std::cout << "Enter your message:";
        for (;;)
        {
            std::string message;
            std::getline(std::cin, message);

            if (message == ":q")
            {
                break;
            }

            send_message(connection, message);
        }

        connection.stop([&task](std::exception_ptr exception)
        {
            try
            {
                if (exception)
                {
                    std::rethrow_exception(exception);
                }

                std::cout << "connection stopped successfully" << std::endl;
            }
            catch (const std::exception & e)
            {
                std::cout << "exception when stopping connection: " << e.what() << std::endl;
            }

            task.set_value();
        });
    });

    task.get_future().get();

    return 0;
}