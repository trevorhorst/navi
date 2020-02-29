#include "http/server/response.h"

namespace NewHttp {

Response::Response( MHD_Connection *mhdConnection )
    : mMhdConnection( mhdConnection )
    , mStatus( Status::OK )
    , mBodyFileDescriptor( -1 )
    , mBodyFileOffset( 0 )
    , mBodyFileSize( 0 )
    , mSent( false )
{

}

void Response::send()
{
    MHD_Response* response;

    if( mSent == false) {
        if( mBodyFileDescriptor >= 0 ) {
            response = MHD_create_response_from_fd_at_offset(
                        mBodyFileSize
                        , mBodyFileDescriptor
                        , mBodyFileOffset );
        } else {
            response = MHD_create_response_from_buffer(
                        mBody.getSize()
                        , (void*) mBody.getData()
                        , MHD_RESPMEM_PERSISTENT );
        }

        for( auto it = mHeaders.begin(); it != mHeaders.end(); ++it ) {
            MHD_add_response_header(
                        response
                        , qPrintable( to_header_case(it.key()))
                        , qPrintable( it.value() ) );
        }

        MHD_queue_response( mMhdConnection, (uint32_t)mStatus, response);
        MHD_destroy_response(response);

        mSent = true;

        // emit sent();
    }

}

}