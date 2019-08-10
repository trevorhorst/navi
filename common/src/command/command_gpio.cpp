#include "common/command/command_gpio.h"

CommandGpio::CommandGpio( const char *mutator, const char *accessor )
    : CommandTemplate< AM335X::Gpio >( mutator, accessor )
    , mPin( 0 )
    , mBank( 0 )
{
    mRequiredMap[ PARAM_BANK ] = PARAMETER_CALLBACK( &CommandGpio::setBank );
    mRequiredMap[ PARAM_PIN ] = PARAMETER_CALLBACK( &CommandGpio::setPin );

    mMutatorMap[ PARAM_OUTPUT ] = PARAMETER_CALLBACK( &CommandGpio::setOutput );
    mMutatorMap[ PARAM_DIR ] = PARAMETER_CALLBACK( &CommandGpio::setDirection );

    mAccessorMap[ PARAM_BANK ] = PARAMETER_CALLBACK( &CommandGpio::getBank );
    mAccessorMap[ PARAM_PIN ] = PARAMETER_CALLBACK( &CommandGpio::getPin );
    mAccessorMap[ PARAM_OUTPUT ] = PARAMETER_CALLBACK( &CommandGpio::getOutput );
    mAccessorMap[ PARAM_INPUT ] = PARAMETER_CALLBACK( &CommandGpio::getInput );
    mAccessorMap[ PARAM_DIR ] = PARAMETER_CALLBACK( &CommandGpio::getDirection );
}

uint32_t CommandGpio::setBank( cJSON *val )
{
    uint32_t r = Error::Code::NONE;
    if( cJSON_IsNumber( val ) ) {
        mBank = static_cast< uint32_t >( val->valueint );
        mControlObject = AM335X::Gpio::getControlObject( mBank );
        if( mControlObject == nullptr ) {
            r = Error::Code::PARAM_OUT_OF_RANGE;
        }
    } else {
        r = Error::Code::SYNTAX;
    }
    return r;
}

uint32_t CommandGpio::setPin( cJSON *val )
{
    uint32_t r = Error::Code::NONE;
    if( cJSON_IsNumber( val ) ) {
        mPin = static_cast< uint32_t >( val->valueint );
    } else {
        r = Error::Code::SYNTAX;
    }
    return r;
}

uint32_t CommandGpio::setOutput( cJSON *val )
{
    uint32_t r = Error::Code::NONE;
    if( cJSON_IsTrue( val ) ) {
        mControlObject->setOutput( mPin, true );
    } else if( cJSON_IsFalse( val ) ) {
        mControlObject->setOutput( mPin, false );
    } else {
        r = Error::Code::SYNTAX;
    }
    return r;
}

uint32_t CommandGpio::setDirection( cJSON *val )
{
    uint32_t r = Error::Code::NONE;
    if( cJSON_IsString( val ) ) {
        mControlObject->setDirection( mPin, val->valuestring );
    } else {
        r = Error::Code::SYNTAX;
    }
    return r;
}

uint32_t CommandGpio::getBank( cJSON *response )
{
    uint32_t r = Error::Code::NONE;
    cJSON_AddNumberToObject( response, PARAM_BANK, mControlObject->getId() );
    return r;
}

uint32_t CommandGpio::getPin( cJSON *response )
{
    uint32_t r = Error::Code::NONE;
    cJSON_AddNumberToObject( response, PARAM_PIN, mPin );
    return r;
}

uint32_t CommandGpio::getOutput( cJSON *response )
{
    uint32_t r = Error::Code::NONE;
    cJSON_AddBoolToObject( response, PARAM_OUTPUT, mControlObject->getOutput( mPin ) );
    return r;
}

uint32_t CommandGpio::getInput( cJSON *response )
{
    uint32_t r = Error::Code::NONE;
    cJSON_AddBoolToObject( response, PARAM_INPUT, mControlObject->getInput( mPin ) );
    return r;
}

uint32_t CommandGpio::getDirection( cJSON *response )
{
    uint32_t r = Error::Code::NONE;
    cJSON_AddStringToObject( response, PARAM_DIR, mControlObject->getDirection( mPin ) );
    return r;
}