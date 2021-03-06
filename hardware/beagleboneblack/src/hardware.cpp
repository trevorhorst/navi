#include "hardware/hardware.h"

const char *Hardware::str_gps_device = "/dev/ttyS1";
const char *Hardware::str_dev_i2c0 = "/dev/i2c-2";
const uint8_t Hardware::ssd1306_address = 0x3C;

/**
 * @brief Constructor
 */
Hardware::Hardware()
    : HardwareBase()
    , mIndexHtml( Resources::load( Resources::index_html, Resources::index_html_size ) )
    , mBundleJs( Resources::load( Resources::bundle_js, Resources::bundle_js_size ) )
    , mGpsSerial( str_gps_device, Serial::Speed::BAUD_9600, isSimulated() )
    , mI2C{ { str_dev_i2c0, ssd1306_address, 0 } }
    , mDisplay( &mI2C[ 0 ], ssd1306_address )
    , mGps( &mGpsSerial )
    , mControlModule( AM335X::addr_control_module, isSimulated() )
    , mClockModule( AM335X::addr_clock_module, isSimulated() )
    , mGpio{ { AM335X::addr_gpio0_base, isSimulated() }
             , { AM335X::addr_gpio1_base, isSimulated() }
             , { AM335X::addr_gpio2_base, isSimulated() }
             , { AM335X::addr_gpio3_base, isSimulated() }
             }
    , mLed{ { &mGpio[ 1 ], 21 }
            , { &mGpio[ 1 ], 22 }
            , { &mGpio[ 1 ], 23 }
            , { &mGpio[ 1 ], 24 }
            }
    , mServer( mIndexHtml, mBundleJs )
    , mHeartbeatTimer( 1000, Timer::Type::INTERVAL, std::bind( &Hardware::heartbeat, this ) )
{
    // Add the individual commands
    addCommand( &mCmdHelp );
    addCommand( &mCmdGpio );
    addCommand( &mCmdHeartbeat );
    addCommand( &mCmdSystem );
    addCommand( &mCmdDateTime );
    addCommand( &mCmdServer );
    addCommand( &mCmdLed );
    addCommand( &mCmdGps );

    // Set the command handler and start the server
    mServer.setCommandHandler( getCommandHandler() );
    mServer.listen();

    mHeartbeatTimer.start();
}

/**
 * @brief Destructor
 */
Hardware::~Hardware()
{
    // Destruct things in the reverse order
    mHeartbeatTimer.stop();

    mServer.stop();
    Resources::unload( mIndexHtml );
    Resources::unload( mBundleJs );
}

Transport::Client *Hardware::getClient()
{
    return &mClient;
}

/**
 * @brief Heartbeat for the hardware, update pertinant hardware information from
 * this method
 */
void Hardware::heartbeat()
{
    // LOG_INFO( "Calling heartbeat" );
    // Periodically flush the serial buffer to keep data flowing
    // mGpsSerial.flushReceiver();

    // Toggle LED for heartbeat
    mLed[ 0 ].setEnable( !mLed[ 0 ].isEnabled() );
}
