add_custom_target(mcuboot_shared_property_target)
set_property(TARGET mcuboot_shared_property_target  PROPERTY KERNEL_HEX_NAME;zephyr.hex)
set_property(TARGET mcuboot_shared_property_target  PROPERTY ZEPHYR_BINARY_DIR;C:/ouroboros-dev/Board_Programs/DEV_KIT_MicroMinutes/build/mcuboot/zephyr)
set_property(TARGET mcuboot_shared_property_target  PROPERTY KERNEL_ELF_NAME;zephyr.elf)
set_property(TARGET mcuboot_shared_property_target  PROPERTY BUILD_BYPRODUCTS;C:/ouroboros-dev/Board_Programs/DEV_KIT_MicroMinutes/build/mcuboot/zephyr/zephyr.hex;C:/ouroboros-dev/Board_Programs/DEV_KIT_MicroMinutes/build/mcuboot/zephyr/zephyr.elf)
set_property(TARGET mcuboot_shared_property_target  PROPERTY SIGNATURE_KEY_FILE;root-rsa-2048.pem)
set_property(TARGET mcuboot_shared_property_target APPEND PROPERTY PM_YML_DEP_FILES;C:/ncs/v2.6.1/bootloader/mcuboot/boot/zephyr/pm.yml)
set_property(TARGET mcuboot_shared_property_target APPEND PROPERTY PM_YML_FILES;C:/ouroboros-dev/Board_Programs/DEV_KIT_MicroMinutes/build/mcuboot/zephyr/include/generated/pm.yml)
