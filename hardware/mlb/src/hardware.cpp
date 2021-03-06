#include "hardware/hardware.h"

/// @brief 60 second delay
const uint32_t Hardware::heartbeat_delay_60000_ms = 60000;

const char *Hardware::mlb_api_host = "http://lookup-service-prod.mlb.com";
const char *Hardware::mlb_api_path = "/json/named.%s.bam?";
const char *Hardware::endpoint_transaction_all  = "transaction_all";
const char *Hardware::endpoint_query_results    = "queryResults";
const char *Hardware::endpoint_total_size       = "totalSize";
const char *Hardware::endpoint_wsfb_news_injury = "wsfb_news_injury";

const char *Hardware::mlb_api =
        "http://lookup-service-prod.mlb.com/json/named.transaction_all.bam?sport_code='mlb'&start_date='%s'&end_date='%s'";
const char *Hardware::smtp_gmail_server = "smtp://smtp.gmail.com:587";

const char *Hardware::email_list[256] = {
    "trevorhorst1212@gmail.com"
    , "trevor.w.horst@gmail.com"
};

const char *Hardware::mlb_api_date = "%Y%m%d";

/**
 * @brief Constructor
 */
Hardware::Hardware()
    : HardwareBase()
    , mIndexHtml( Resources::load( Resources::index_html, Resources::index_html_size ) )
    , mBundleJs( Resources::load( Resources::bundle_js, Resources::bundle_js_size ) )
    , mServer( mIndexHtml, mBundleJs )
    , mHeartbeatTimer( heartbeat_delay_60000_ms
                       , Timer::Type::INTERVAL
                       , std::bind( &Hardware::heartbeat, this ) )
{
    // Configure the SMTP client
    // mSmtpClient.setUsername( "Put username here" );
    // mSmtpClient.setPassword( "Put password here" );
    // mSmtpClient.addTo( "Add recipient here" );

    mSmtpClient.setServer( smtp_gmail_server );
    mSmtpClient.setReadFunction( &Smtp::Client::readFunction );
    mSmtpClient.setSubject( "MLB Transactions" );
    mSmtpClient.applySettings();


    // Add the individual commands
    addCommand( &mCmdHelp );
    addCommand( &mCmdHeartbeat );
    addCommand( &mCmdSystem );
    addCommand( &mCmdDateTime );
    addCommand( &mCmdServer );
    addCommand( &mCmdSmtp );

    // Set the command handler and start the server
    mServer.setCommandHandler( getCommandHandler() );
    mServer.listen();

    testApi();

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

/**
 * @brief Returns a pointer to hardware transport client for communication
 * @return Transport::Client pointer
 */
Transport::Client *Hardware::getClient()
{
    return &mHttpClient;
}

/**
 * @brief Heartbeat for the hardware, update pertinant hardware information from
 * this method
 */
void Hardware::heartbeat()
{
    // char date[ 9 ] = "20171201";
    // getDate( date, 9 );
    // queryTransactions( date, date );
}

/**
 * @brief Gets a date string in the format YYYYMMDD
 * @param buffer Container to store date data
 * @param size Size of container
 * @return Error code
 */
uint32_t Hardware::getDate( char *buffer, size_t size )
{
    uint32_t error = Error::Code::NONE;

    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime( &rawtime );

    strftime( buffer, size, mlb_api_date, timeinfo );

    return error;
}

/**
 * @brief Query MLB transactions
 * @param startDate Query start date
 * @param stopDate Query stop date
 * @return Error code
 */
uint32_t Hardware::queryTransactions( const char *startDate, const char *stopDate )
{
    uint32_t error = Error::Code::NONE;

    // char date[ 9 ] = "20171201";
    // getDate( date, 19);
    char getUrl[512];

    if( startDate == nullptr || stopDate == nullptr ) {
        error = Error::Code::GENERIC;
        LOG_WARN( "Start date or stop date is invalid\n" );
    } else {
        snprintf( getUrl, 256, mlb_api, startDate, stopDate );
        mMlbApiClient.applyUrl( getUrl );
    }

    if( !error ) {
        // Parse the information retrieved from the site
        cJSON *parsed = cJSON_Parse( mMlbApiClient.get().c_str() );
        cJSON *transactionAll = cJSON_GetObjectItem( parsed, endpoint_transaction_all );
        cJSON *queryResults = cJSON_GetObjectItem( transactionAll, endpoint_query_results );
        cJSON *totalSize    = cJSON_GetObjectItem( queryResults, endpoint_total_size );
        if( cJSON_IsString( totalSize ) ) {
            int32_t size = atoi( totalSize->valuestring );
            printf( "TotalSize: %d\n",  size );
            cJSON *row = cJSON_GetObjectItem( queryResults, "row" );
            if( cJSON_IsArray( row ) ) {
                for( int32_t i = 0; i < size; i++ ) {
                    cJSON *item = cJSON_GetArrayItem( row, i );
                    char *printItem = cJSON_Print( item );
                    // printf( "%s\n", printItem );
                    mSmtpClient.send( printItem );
                    free( printItem );
                }
            }
        }
        cJSON_Delete( parsed );
    }

    return error;
}

uint32_t Hardware::testApi()
{
    uint32_t error = Error::Code::NONE;
    char date[ 9 ] = "20171201";
    getDate( date, 9 );
    queryTransactions( date, date );
    // queryTransactions( "20171201", "20171201" );
    return error;
}
