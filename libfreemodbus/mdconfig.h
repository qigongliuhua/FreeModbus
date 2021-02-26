#ifndef __MDCONFIG_H__
#define __MDCONFIG_H__

#define DEBUG                       (1)

#define IGNORE_LOSS_FRAME           (1)
#define IGNORE_CRC_CHECK            (1)


#define MODBUS_PDU_SIZE_MIN         (4)
#define MODBUS_PDU_SIZE_MAX         (253)

#define REGISTER_WIDTH              (16)


#define REGISTER_OFFSET             (1)
#define COIL_OFFSET                         (0)
#define INPUT_COIL_OFFSET                   (10000)
#define INPUT_REGISTER_OFFSET               (30000)
#define KEEP_REGISTER_OFFSET                (40000)
#define REGISTER_POOL_MAX_BUFFER            (16)

#define REGISTER_POOL_MAX_REGISTER_NUMBER   (256)

#endif