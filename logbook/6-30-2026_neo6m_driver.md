start developing the neo6m driver
update the coding guidelines
ensure that the coding guidelines are enforces consistently accross all drivers
make sure that all the drivers are using HAL errors
remember to get rid of "magic numbers" in all drivers

discuss the differnece between having a data pointer for a driver versus caching data internally
for an i2c driver where polling is required, make it a data pointer so the user can get the data when they want
for a streaming interface like UART, where the data may not always be ready, cache the data internally

consider adding driver specific error return codes