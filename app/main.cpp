#include <thread>
#include <iostream>

#include "common/console/console.h"
#include "common/command/command.h"
#include "common/http/server/server.h"
#include "common/http/client/client.h"
#include "common/system/datetime.h"
#include "beagleboneblack/hardware_beagleboneblack.h"

#include "common/command/command_console.h"
#include "common/command/command_system.h"
#include "common/command/command_datetime.h"
#include "common/command/command_help.h"

/**
 * @brief Entry point of the program
 * @return
 */
int main()
{
    CommandHandler commandHandler;

    // Create the console
    Console *console = &Console::getInstance();
    CommandConsole cmdConsole;
    commandHandler.addCommand( &cmdConsole );

    // Create the system
    System sys;
    CommandSystem cmdSystem;
    commandHandler.addCommand( &cmdSystem );

    // Create the datetime
    DateTime dateTime;
    CommandDateTime cmdDateTime;
    commandHandler.addCommand( &cmdDateTime );

    // Create the hardware
    BeagleboneBlack *bbb = &BeagleboneBlack::getInstance();
    (void)bbb;

    // Create the server
    HttpServer server( Resources::INDEX_HTML, Resources::MAIN_JS );
    server.setCommandHandler( &commandHandler );
    server.listen();

    CommandHelp help;
    commandHandler.addCommand( &help );

    std::thread *app = new std::thread( &Console::run, console );
    // std::thread *req = new std::thread( &HttpServer::handleRequests, &server );
    // req->detach();
    app->join();

    server.mDone = true;

    server.stop();

    delete app;

    return 0;
}
