#include "smtp/client.h"

namespace Smtp {

uint32_t Client::set_counter        = 0;
const uint32_t Client::set_server   = COMMON_BITFIELD( set_counter++, 1 );
const uint32_t Client::set_username = COMMON_BITFIELD( set_counter++, 1 );
const uint32_t Client::set_password = COMMON_BITFIELD( set_counter++, 1 );
const uint32_t Client::set_subject  = COMMON_BITFIELD( set_counter++, 1 );
const uint32_t Client::set_recipients = COMMON_BITFIELD( set_counter++, 1 );
const uint32_t Client::set_read_function = COMMON_BITFIELD( set_counter++, 1 );

const char *Client::date_skeleton = "Date: %a, %d %h %G %X %z\r\n";

/**
 * @brief Constructor
 */
Client::Client()
    : mCurl( nullptr )
    , mRecipients( nullptr )
{
    mCurl = curl_easy_init();
}

/**
 * @brief Destructor
 */
Client::~Client()
{
    close();
}

/**
 * @brief Cleans up the SMTP connection
 */
void Client::close()
{
    clearRecipients();
    curl_easy_cleanup( mCurl );
    mCurl = nullptr;
}

/**
 * @brief Retrieves the destination server
 * @return String representation of the server name
 */
std::string Client::getServer()
{
    const char *url = nullptr;
    curl_easy_getinfo( mCurl, CURLINFO_EFFECTIVE_URL, &url );
    return url;
}

const curl_slist *Client::getRecipientsList() const
{
    return mRecipients;
}

/**
 * @brief Adds a 'To' recipient to the recipient list
 * @param recipient Desired recipient address
 * @return Error code
 */
uint32_t Client::addTo( const std::string recipient )
{
    uint32_t error = Error::Code::NONE;
    if( recipient.empty() ) {
        error = Error::Code::PARAM_OUT_OF_RANGE;
    } else {
        mSettings.toList.push_back( recipient );
        mSettings.mask |= set_recipients;
    }
    return error;
}

/**
 * @brief Adds a 'Cc' recipient to the recipient list
 * @param recipient Desired recipient list
 * @return Error code
 */
uint32_t Client::addCarbonCopy( const std::string recipient )
{
    uint32_t error = Error::Code::NONE;
    if( recipient.empty() ) {
        error = Error::Code::PARAM_OUT_OF_RANGE;
    } else {
        mSettings.carbonCopyList.push_back( recipient );
        mSettings.mask |= set_recipients;
    }
    return error;
}

uint32_t Client::clearRecipients()
{
    uint32_t error = Error::Code::NONE;
    if( mRecipients ) {
        curl_slist_free_all( mRecipients );
        mRecipients = nullptr;
    }
    return error;
}

/**
 * @brief Sets the target SMTP server
 * @param server Desired server
 * @return Error code
 */
uint32_t Client::setServer( const std::string &server )
{
    uint32_t error = Error::Code::NONE;
    mSettings.server = server;
    mSettings.mask |= set_server;
    return error;
}

/**
 * @brief Sets the username
 * @param username Desired username
 * @return Error code
 */
uint32_t Client::setUsername(const std::string &username)
{
    uint32_t error = Error::Code::NONE;
    mSettings.username = username;
    mSettings.mask |= set_username;
    return error;
}

/**
 * @brief Sets the password
 * @param password Desired password
 * @return Error code
 */
uint32_t Client::setPassword(const std::string &password)
{
    uint32_t error = Error::Code::NONE;
    mSettings.password = password;
    mSettings.mask |= set_password;
    return error;
}

/**
 * @brief Sets the subject tag for the message
 * @param subject Desired subject tag
 * @return Error code
 */
uint32_t Client::setSubject( const std::string &subject )
{
    uint32_t error = Error::Code::NONE;
    mSettings.subject = subject;
    mSettings.mask |= set_subject;
    return error;
}

/**
 * @brief Sets the callback read function
 * @param readFunction Desired callback read function
 * @return Error code
 */
uint32_t Client::setReadFunction(ReadFunction *readFunction)
{
    uint32_t error = Error::Code::NONE;
    if( readFunction != nullptr ) {
        mSettings.readFunction = readFunction;
        mSettings.mask |= set_read_function;
    }
    return error;
}

/**
 * @brief Sends a desired message with configured SMTP settings
 * @param message Desired message to send
 * @return Error code
 */
uint32_t Client::send( const std::string &message )
{
    CURLcode res = CURLE_OK;

    mPayload.sent = 0;
    applyDate();
    mPayload.message = message;
    mPayload.message.append( "\r\n" );

    if( mCurl ) {
        curl_easy_setopt(mCurl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

        curl_easy_setopt(mCurl, CURLOPT_READDATA, &mPayload);
        curl_easy_setopt(mCurl, CURLOPT_UPLOAD, 1L);

        res = curl_easy_perform(mCurl);

        /* Check for errors */
        if( res != CURLE_OK ) {
            fprintf( stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
    }

    mPayload.sent = 0;
    return 0;
}

/**
 * @brief Callback function to handle sending different segments of the message
 * @param pointer
 * @param size
 * @param nmemb
 * @param userp
 * @return
 */
size_t Client::readFunction( void *pointer, size_t size, size_t nmemb, void *userp )
{
    Payload *payload = reinterpret_cast< Payload* >( userp );

    if( ( size == 0 )
            || (nmemb == 0)
            || ( (size*nmemb) < 1 ) ) {
      return 0;
    }

    /// @todo Need a more robust way of setting up the data to send out
    std::string *data = nullptr;
    if( payload->sent == 0 ) {
        data = &payload->date;
    } else if( payload->sent == 1 ) {
        data = &payload->to;
    } else if( payload->sent == 2 ) {
        data = &payload->cc;
    } else if( payload->sent == 3 ) {
        data = &payload->subject;
    } else if( payload->sent == 4 ) {
        data = &payload->message;
    }

    if( data ) {
        LOG_INFO( "length: %lu", (*data).length() );
        LOG_INFO( "data: %s", (*data).c_str() );

        size_t len = data->length();
        memcpy( pointer, data->c_str(), len );
        payload->sent++;
        return len;
    }

    return 0;
}

/**
 * @brief Applies date to the payload
 * @return Error code
 */
uint32_t Client::applyDate()
{
    uint32_t error = Error::Code::NONE;

    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];

    time (&rawtime);
    timeinfo = localtime( &rawtime );

    strftime( buffer, 80, date_skeleton, timeinfo );
    mPayload.date = buffer;

    return error;
}

/**
 * @brief Applies the server to the cURL object
 * @param server Reference to the desired server
 * @return Error code
 */
uint32_t Client::applyServer( const std::string &server )
{
    uint32_t error = Error::Code::NONE;
    curl_easy_setopt( mCurl, CURLOPT_URL, server.c_str() );
    return error;
}

/**
 * @brief Applies the username to the cURL object
 * @param username Reference to the desired username
 * @return Error code
 */
uint32_t Client::applyUsername( const std::string &username )
{
    uint32_t error = Error::Code::NONE;
    curl_easy_setopt( mCurl, CURLOPT_USERNAME, username.c_str() );
    curl_easy_setopt( mCurl, CURLOPT_MAIL_FROM, username.c_str() );
    return error;
}

/**
 * @brief Applies the password to the cURL object
 * @param password Reference to the desired password
 * @return Error code
 */
uint32_t Client::applyPassword( const std::string &password )
{
    uint32_t error = Error::Code::NONE;
    curl_easy_setopt( mCurl, CURLOPT_PASSWORD, password.c_str() );
    return error;
}

/**
 * @brief Apply recipients to the cURL object
 * @param toList Reference to the 'To' list
 * @param ccList Reference eo the 'Cc' list
 * @return Error code
 */
uint32_t Client::applyRecipients( const std::vector< std::string > &toList
                                  , const std::vector< std::string > &ccList )
{
    uint32_t error = Error::Code::NONE;

    mPayload.to = "To:";
    for( auto i = toList.begin(); i != toList.end(); i++ ) {
        mRecipients = curl_slist_append( mRecipients, (*i).c_str() );
        mPayload.to.append( " <" ).append(  *i ).append( ">" );
    }
    mPayload.to.append( " \r\n" );

    mPayload.cc = "Cc:";
    for( auto i = ccList.begin(); i != ccList.end(); i++ ) {
        mRecipients = curl_slist_append( mRecipients, (*i).c_str() );
        mPayload.cc.append( " <" ).append( *i ).append( ">" );
    }
    mPayload.cc.append( " \r\n" );

    curl_easy_setopt( mCurl, CURLOPT_MAIL_RCPT, mRecipients );

    return error;
}

/**
 * @brief Applies a subject to the cURL object
 * @param subject Reference to the desired subject
 * @return Error code
 */
uint32_t Client::applySubject( const std::string &subject )
{
    uint32_t error = Error::Code::NONE;
    mPayload.subject = "Subject: ";
    mPayload.subject.append( subject );
    mPayload.subject.append( "\r\n" );
    return error;
}

/**
 * @brief Applies the callback read fucntion to the cURL object
 * @param readFunction Pointer to the desired readback function
 * @return Error code
 */
uint32_t Client::applyReadFunction(ReadFunction *readFunction)
{
    uint32_t error = Error::Code::NONE;
    curl_easy_setopt( mCurl, CURLOPT_READFUNCTION, *readFunction );
    return error;
}

/**
 * @brief Applies all flagged settings
 * @return Error code
 */
uint32_t Client::applySettings()
{
    uint32_t error = Error::Code::NONE;

    if( mCurl ) {
        if( ( error == Error::Code::NONE ) && mSettings.mask & set_server ) {
            error = applyServer( mSettings.server );
        }

        if( ( error == Error::Code::NONE ) && mSettings.mask & set_username ) {
            error = applyUsername( mSettings.username );
        }

        if( ( error == Error::Code::NONE && mSettings.mask & set_password ) ) {
            error = applyPassword( mSettings.password );
        }

        if( ( error == Error::Code::NONE && mSettings.mask & set_password ) ) {
            error = applyReadFunction( mSettings.readFunction );
        }

        if( ( error == Error::Code::NONE && mSettings.mask & set_subject ) ) {
            error = applySubject( mSettings.subject );
        }

        if( ( error == Error::Code::NONE && mSettings.mask & set_recipients ) ) {
            error = applyRecipients( mSettings.toList, mSettings.carbonCopyList );
        }

        mSettings.toList.clear();
        mSettings.carbonCopyList.clear();
        mSettings.mask = 0;

    } else {
        error = Error::Code::CONTROL_MISSING;
    }

    return error;
}

}
