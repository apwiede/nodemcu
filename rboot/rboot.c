//////////////////////////////////////////////////
// rBoot open source boot loader for ESP8266.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
//////////////////////////////////////////////////

#ifdef RBOOT_INTEGRATION
#include <rboot-integration.h>
#endif

#include "rboot-private.h"
#include <rboot-hex2a.h>

static char hexchars[] = "0123456789ABCDEF";
static char valstr[50];
static char valstr2[50];

static uint32_t check_image(uint32_t readpos) {
  uint8_t buffer[BUFFER_SIZE];
  uint8_t sectcount;
  uint8_t sectcurrent;
  uint8_t *writepos;
  uint8_t chksum;
  uint32_t loop;
  uint32_t remaining;
  uint32_t romaddr;

  chksum = CHKSUM_INIT;
  rom_header_new *header = (rom_header_new*)buffer;
  section_header *section = (section_header*)buffer;

  if (readpos == 0 || readpos == 0xffffffff) {
    ets_printf("fail 1\r\n");
    return 0;
  }

  // read rom header
  if (SPIRead(readpos, header, sizeof(rom_header_new)) != 0) {
    ets_printf("fail 2\r\n");
    return 0;
  }

  // check header type
ets_printf("readpos: 0x%08x magic: 0x%02x\n", readpos, header->magic & 0xFF);
  if (header->magic == ROM_MAGIC) {
    // old type, no extra header or irom section to skip over
    romaddr = readpos;
    readpos += sizeof(rom_header);
    sectcount = header->count;
  } else if (header->magic == ROM_MAGIC_NEW1 && header->count == ROM_MAGIC_NEW2) {
    // new type, has extra header and irom section first
    romaddr = readpos + header->len + sizeof(rom_header_new);
#ifdef BOOT_IROM_CHKSUM
    // we will set the real section count later, when we read the header
    sectcount = 0xff;
    // just skip the first part of the header
    // rest is processed for the chksum
    readpos += sizeof(rom_header);
#else
    // skip the extra header and irom section
    readpos = romaddr;
    // read the normal header that follows
    if (SPIRead(readpos, header, sizeof(rom_header)) != 0) {
      ets_printf("fail 3\r\n");
      return 0;
    }
    sectcount = header->count;
    readpos += sizeof(rom_header);
#endif
  } else {
    ets_printf("fail 4\r\n");
    return 0;
  }

  // test each section
  for (sectcurrent = 0; sectcurrent < sectcount; sectcurrent++) {
    // read section header
    if (SPIRead(readpos, section, sizeof(section_header)) != 0) {
      ets_printf("fail 5\r\n");
      return 0;
    }
    readpos += sizeof(section_header);

    // get section address and length
    writepos = section->address;
    remaining = section->length;

    while (remaining > 0) {
      // work out how much to read, up to BUFFER_SIZE
      uint32_t readlen = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;
      // read the block
      if (SPIRead(readpos, buffer, readlen) != 0) {
        ets_printf("fail 6\r\n");
        return 0;
      }
      // increment next read and write positions
      readpos += readlen;
      writepos += readlen;
      // decrement remaining count
      remaining -= readlen;
      // add to chksum
      for (loop = 0; loop < readlen; loop++) {
        chksum ^= buffer[loop];
      }
    }

#ifdef BOOT_IROM_CHKSUM
    if (sectcount == 0xff) {
      // just processed the irom section, now
      // read the normal header that follows
      if (SPIRead(readpos, header, sizeof(rom_header)) != 0) {
        ets_printf("fail 7\r\n");
        return 0;
      }
      sectcount = header->count + 1;
      readpos += sizeof(rom_header);
    }
#endif
  }

  // round up to next 16 and get checksum
  readpos = readpos | 0x0f;
  if (SPIRead(readpos, buffer, 1) != 0) {
    ets_printf("fail 8\r\n");
    return 0;
  }

  // compare calculated and stored checksums
  if (buffer[0] != chksum) {
    ets_printf("fail 9 buffer[0]: 0x%02x chksum: 0x%02x\r\n", buffer[0], chksum);
    return 0;
  }

  return romaddr;
}

#ifdef BOOT_GPIO_ENABLED

#if BOOT_GPIO_NUM > 16
#error "Invalid BOOT_GPIO_NUM value (disable BOOT_GPIO_ENABLED to disable this feature)"
#endif

// sample gpio code for gpio16
#define ETS_UNCACHED_ADDR(addr) (addr)
#define READ_PERI_REG(addr) (*((volatile uint32_t *)ETS_UNCACHED_ADDR(addr)))
#define WRITE_PERI_REG(addr, val) (*((volatile uint32_t *)ETS_UNCACHED_ADDR(addr))) = (uint32_t)(val)
#define PERIPHS_RTC_BASEADDR        0x60000700
#define REG_RTC_BASE  PERIPHS_RTC_BASEADDR
#define RTC_GPIO_OUT              (REG_RTC_BASE + 0x068)
#define RTC_GPIO_ENABLE              (REG_RTC_BASE + 0x074)
#define RTC_GPIO_IN_DATA            (REG_RTC_BASE + 0x08C)
#define RTC_GPIO_CONF              (REG_RTC_BASE + 0x090)
#define PAD_XPD_DCDC_CONF            (REG_RTC_BASE + 0x0A0)
static uint32_t get_gpio16(void) {
  // set output level to 1
  WRITE_PERI_REG(RTC_GPIO_OUT, (READ_PERI_REG(RTC_GPIO_OUT) & (uint32_t)0xfffffffe) | (uint32_t)(1));

  // read level
  WRITE_PERI_REG(PAD_XPD_DCDC_CONF, (READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32_t)0x1);  // mux configuration for XPD_DCDC and rtc_gpio0 connection
  WRITE_PERI_REG(RTC_GPIO_CONF, (READ_PERI_REG(RTC_GPIO_CONF) & (uint32_t)0xfffffffe) | (uint32_t)0x0);  //mux configuration for out enable
  WRITE_PERI_REG(RTC_GPIO_ENABLE, READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32_t)0xfffffffe);  //out disable

  return (READ_PERI_REG(RTC_GPIO_IN_DATA) & 1);
}

// support for "normal" GPIOs (other than 16)
#define REG_GPIO_BASE            0x60000300
#define GPIO_IN_ADDRESS          (REG_GPIO_BASE + 0x18)
#define GPIO_ENABLE_OUT_ADDRESS  (REG_GPIO_BASE + 0x0c)
#define REG_IOMUX_BASE           0x60000800
#define IOMUX_PULLUP_MASK        (1<<7)
#define IOMUX_FUNC_MASK          0x0130
const uint8_t IOMUX_REG_OFFS[] = {0x34, 0x18, 0x38, 0x14, 0x3c, 0x40, 0x1c, 0x20, 0x24, 0x28, 0x2c, 0x30, 0x04, 0x08, 0x0c, 0x10};
const uint8_t IOMUX_GPIO_FUNC[] = {0x00, 0x30, 0x00, 0x30, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};

static int get_gpio(int gpio_num) {
  // disable output buffer if set
  uint32_t old_out = READ_PERI_REG(GPIO_ENABLE_OUT_ADDRESS);
  uint32_t new_out = old_out & ~ (1<<gpio_num);
  WRITE_PERI_REG(GPIO_ENABLE_OUT_ADDRESS, new_out);

  // set GPIO function, enable soft pullup
  uint32_t iomux_reg = REG_IOMUX_BASE + IOMUX_REG_OFFS[gpio_num];
  uint32_t old_iomux = READ_PERI_REG(iomux_reg);
  uint32_t gpio_func = IOMUX_GPIO_FUNC[gpio_num];
  uint32_t new_iomux = (old_iomux & ~IOMUX_FUNC_MASK) | gpio_func | IOMUX_PULLUP_MASK;
  WRITE_PERI_REG(iomux_reg, new_iomux);

  // allow soft pullup to take effect if line was floating
  ets_delay_us(10);
  int result = READ_PERI_REG(GPIO_IN_ADDRESS) & (1<<gpio_num);

  // set iomux & GPIO output mode back to initial values
  WRITE_PERI_REG(iomux_reg, old_iomux);
  WRITE_PERI_REG(GPIO_ENABLE_OUT_ADDRESS, old_out);
  return (result ? 1 : 0);
}

// return '1' if we should do a gpio boot
static int perform_gpio_boot(rboot_config *romconf) {
  if (romconf->mode & MODE_GPIO_ROM == 0) {
    return FALSE;
  }

  // pin low == GPIO boot
  if (BOOT_GPIO_NUM == 16) {
    return (get_gpio16() == 0);
  } else {
    return (get_gpio(BOOT_GPIO_NUM) == 0);
  }
}

#endif

#ifdef BOOT_RTC_ENABLED
uint32_t system_rtc_mem(int32_t addr, void *buff, int32_t length, uint32_t mode) {
  int32_t blocks;

  // validate reading a user block
  if (addr < 64) return 0;
  if (buff == 0) return 0;
  // validate 4 byte aligned
  if (((uint32_t)buff & 0x3) != 0) return 0;
  // validate length is multiple of 4
  if ((length & 0x3) != 0) return 0;

  // check valid length from specified starting point
  if (length > (0x300 - (addr * 4))) return 0;

  // copy the data
  for (blocks = (length >> 2) - 1; blocks >= 0; blocks--) {
    volatile uint32_t *ram = ((uint32_t*)buff) + blocks;
    volatile uint32_t *rtc = ((uint32_t*)0x60001100) + addr + blocks;
    if (mode == RBOOT_RTC_WRITE) {
      *rtc = *ram;
    } else {
      *ram = *rtc;
    }
  }

  return 1;
}
#endif

#if defined(BOOT_CONFIG_CHKSUM) || defined(BOOT_RTC_ENABLED)
// calculate checksum for block of data
// from start up to (but excluding) end
static uint8_t calc_chksum(uint8_t *start, uint8_t *end) {
  uint8_t chksum = CHKSUM_INIT;
  while(start < end) {
    chksum ^= *start;
    start++;
  }
  return chksum;
}
#endif

#ifdef BOOT_AES128_CBC
const uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
const uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
const uint8_t in[]  = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
                    0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
                    0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
                    0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10 };
const uint8_t out[] = { 0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46, 0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d,
                    0x50, 0x86, 0xcb, 0x9b, 0x50, 0x72, 0x19, 0xee, 0x95, 0xdb, 0x11, 0x3a, 0x91, 0x76, 0x78, 0xb2,
                    0x73, 0xbe, 0xd6, 0xb8, 0xe3, 0xc1, 0x74, 0x3b, 0x71, 0x16, 0xe6, 0x9e, 0x22, 0x22, 0x95, 0x16,
                    0x3f, 0xf1, 0xca, 0xa1, 0x68, 0x1f, 0xac, 0x09, 0x12, 0x0e, 0xca, 0x30, 0x75, 0x86, 0xe1, 0xa7 };
static void test_encrypt_cbc(void)
{
  uint8_t buffer[64];

ets_printf("test_encrypt_cbc\r\n");
  AES128_CBC_encrypt_buffer(buffer, in, 64, key, iv);
  ets_printf("\r\nCBC encrypt: ");
  if(strncmp((char*) out, (char*) buffer, 64) == 0) {
    ets_printf("SUCCESS!\r\n");
  } else {
    ets_printf("FAILURE!\r\n");
  }
}
#endif

#ifndef BOOT_CUSTOM_DEFAULT_CONFIG
// populate the user fields of the default config
// created on first boot or in case of corruption
static uint8_t default_config(rboot_config *romconf, uint32_t flashsize) {
  romconf->count = 2;
  romconf->roms[0] = SECTOR_SIZE * (BOOT_CONFIG_SECTOR + 1);
  romconf->roms[1] = (flashsize / 2) + (SECTOR_SIZE * (BOOT_CONFIG_SECTOR + 1));
  romconf->user_rom_save_data_flag = BOOT_USER_ROM_NO_DATA_SET;
  romconf->user_rom_save_data_size = 0;
  romconf->user_rom_save_data[0] = '\0';
}
#endif

// prevent this function being placed inline with main
// to keep main's stack size as small as possible
// don't mark as static or it'll be optimised out when
// using the assembler stub
uint32_t NOINLINE find_image(void) {

  uint8_t flag;
  uint32_t runAddr;
  uint32_t flashsize;
  int32_t romToBoot;
  uint8_t updateConfig = FALSE;
  uint8_t buffer[SECTOR_SIZE];
  uint8_t msg[100];
#ifdef BOOT_GPIO_ENABLED
  uint8_t gpio_boot = FALSE;
  uint8_t sec;
#endif
#ifdef BOOT_RTC_ENABLED
  rboot_rtc_data rtc;
  uint8 temp_boot = FALSE;
#endif

  rboot_config *romconf = (rboot_config*)buffer;
  rom_header *header = (rom_header*)buffer;

#if defined BOOT_DELAY_MICROS && BOOT_DELAY_MICROS > 0
  // delay to slow boot (help see messages when debugging)
  ets_delay_us(BOOT_DELAY_MICROS);
#endif

  ets_printf("\r\nrBoot v1.4.1 - richardaburton@gmail.com\r\n");

  // read rom header
  SPIRead(0, header, sizeof(rom_header));

  // print and get flash size
  ets_printf("Flash Size:   ");
  flag = header->flags2 >> 4;
  if (flag == 0) {
    ets_printf("4 Mbit\r\n");
    flashsize = 0x80000;
  } else if (flag == 1) {
    ets_printf("2 Mbit\r\n");
    flashsize = 0x40000;
  } else if (flag == 2) {
    ets_printf("8 Mbit\r\n");
    flashsize = 0x100000;
  } else if (flag == 3) {
    ets_printf("16 Mbit\r\n");
#ifdef BOOT_BIG_FLASH
    flashsize = 0x200000;
#else
    flashsize = 0x100000; // limit to 8Mbit
#endif
  } else if (flag == 4) {
    ets_printf("32 Mbit\r\n");
#ifdef BOOT_BIG_FLASH
    flashsize = 0x400000;
#else
    flashsize = 0x100000; // limit to 8Mbit
#endif
  } else {
    ets_printf("unknown\r\n");
    // assume at least 4mbit
    flashsize = 0x80000;
  }

  // print spi mode
  ets_printf("Flash Mode:   ");
  if (header->flags1 == 0) {
    ets_printf("QIO\r\n");
  } else if (header->flags1 == 1) {
    ets_printf("QOUT\r\n");
  } else if (header->flags1 == 2) {
    ets_printf("DIO\r\n");
  } else if (header->flags1 == 3) {
    ets_printf("DOUT\r\n");
  } else {
    ets_printf("unknown\r\n");
  }

  // print spi speed
  ets_printf("Flash Speed:  ");
  flag = header->flags2 & 0x0f;
  if (flag == 0) ets_printf("40 MHz\r\n");
  else if (flag == 1) ets_printf("26.7 MHz\r\n");
  else if (flag == 2) ets_printf("20 MHz\r\n");
  else if (flag == 0x0f) ets_printf("80 MHz\r\n");
  else ets_printf("unknown\r\n");

  // print enabled options
#ifdef BOOT_BIG_FLASH
  ets_printf("rBoot Option: Big flash\r\n");
#endif
#ifdef BOOT_CONFIG_CHKSUM
  ets_printf("rBoot Option: Config chksum\r\n");
#endif
#ifdef BOOT_GPIO_ENABLED
  ets_printf("rBoot Option: GPIO mode (%d)\r\n", BOOT_GPIO_NUM);
#endif
#ifdef BOOT_RTC_ENABLED
  ets_printf("rBoot Option: RTC data\r\n");
#endif
#ifdef BOOT_IROM_CHKSUM
  ets_printf("rBoot Option: irom chksum\r\n");
#endif
  ets_printf("\r\n");

#ifdef BOOT_AES128_CBC
  // support for AES128 CBC encyption
  test_encrypt_cbc();
#endif
  // read boot config
  SPIRead(BOOT_CONFIG_SECTOR * SECTOR_SIZE, buffer, SECTOR_SIZE);
  // fresh install or old version?
  if (romconf->magic != BOOT_CONFIG_MAGIC || romconf->version != BOOT_CONFIG_VERSION
#ifdef BOOT_CONFIG_CHKSUM
    || romconf->chksum != calc_chksum((uint8_t*)romconf, (uint8_t*)&romconf->chksum)
#endif
    ) {
    // create a default config for a standard 2 rom setup
    ets_printf("Writing default boot config.\r\n");
    ets_memset(romconf, 0x00, sizeof(rboot_config));
    romconf->magic = BOOT_CONFIG_MAGIC;
    romconf->version = BOOT_CONFIG_VERSION;
    default_config(romconf, flashsize);
#ifdef BOOT_CONFIG_CHKSUM
    romconf->chksum = calc_chksum((uint8_t*)romconf, (uint8_t*)&romconf->chksum);
#endif
    // write new config sector
    SPIEraseSector(BOOT_CONFIG_SECTOR);
    SPIWrite(BOOT_CONFIG_SECTOR * SECTOR_SIZE, buffer, SECTOR_SIZE);
  }

  // try rom selected in the config, unless overriden by gpio/temp boot
  romToBoot = romconf->current_rom;

#ifdef BOOT_RTC_ENABLED
  // if rtc data enabled, check for valid data
  if (system_rtc_mem(RBOOT_RTC_ADDR, &rtc, sizeof(rboot_rtc_data), RBOOT_RTC_READ) &&
    (rtc.chksum == calc_chksum((uint8_t*)&rtc, (uint8_t*)&rtc.chksum))) {

    if (rtc.next_mode & MODE_TEMP_ROM) {
      if (rtc.temp_rom >= romconf->count) {
        ets_printf("Invalid temp rom selected.\r\n");
        return 0;
      }
      ets_printf("Booting temp rom.\r\n");
      temp_boot = TRUE;
      romToBoot = rtc.temp_rom;
    }
  }
#endif

ets_printf("romconf size: %d\n", sizeof(rboot_config));
ets_printf("user_rom_save_data_flag: %d len: %d\n", romconf->user_rom_save_data_flag, romconf->user_rom_save_data_size);
  switch (romconf->user_rom_save_data_flag) {
  case BOOT_USER_ROM_NO_DATA_SET:
    break;
  case BOOT_USER_ROM_DATA_SET:
    break;
  case BOOT_USER_ROM_DO_SAVE_DATA:
    break;
  case BOOT_USER_ROM_NO_SAVE_DATA:
    break;
  case BOOT_USER_ROM_RESET_DATA:
    // clear user_save_data
    romconf->user_rom_save_data_flag = 0;
    ets_memset(romconf->user_rom_save_data, '\0', romconf->user_rom_save_data_size);
    romconf->user_rom_save_data_size = 0;
    updateConfig = TRUE;
    break;
  case BOOT_USER_ROM_DATA_RESETTED:
    break;
  case BOOT_USER_ROM_DATA_SAVED:
    break;
  default:
    break;
  }
  
#ifdef BOOT_GPIO_ENABLED
  if (perform_gpio_boot(romconf)) {
    if (romconf->gpio_rom >= romconf->count) {
      ets_printf("Invalid GPIO rom selected.\r\n");
      return 0;
    }
    ets_printf("Booting GPIO-selected rom.\r\n");
    if (romconf->mode & MODE_GPIO_ERASES_SDKCONFIG) {
      ets_printf("Erasing SDK config sectors before booting.\r\n");
      for (sec = 1; sec < 5; sec++) {
        SPIEraseSector((flashsize / SECTOR_SIZE) - sec);
      }
    }
    romToBoot = romconf->gpio_rom;
    gpio_boot = TRUE;
    updateConfig = TRUE;
  }
#endif

  // check valid rom number
  // gpio/temp boots will have already validated this
  if (romconf->current_rom >= romconf->count) {
    // if invalid rom selected try rom 0
    ets_printf("Invalid rom selected, defaulting to 0.\r\n");
    romToBoot = 0;
    romconf->current_rom = 0;
    updateConfig = TRUE;
  }

  // check rom is valid
ets_printf("check_image: rom: %d addr: 0x%08x\n", romToBoot, romconf->roms[romToBoot]);
  runAddr = check_image(romconf->roms[romToBoot]);

#ifdef BOOT_GPIO_ENABLED
  if (gpio_boot && runAddr == 0) {
    // don't switch to backup for gpio-selected rom
    ets_printf("GPIO boot rom (%d) is bad.\r\n", romToBoot);
    return 0;
  }
#endif
#ifdef BOOT_RTC_ENABLED
  if (temp_boot && runAddr == 0) {
    // don't switch to backup for temp rom
    ets_printf("Temp boot rom (%d) is bad.\r\n", romToBoot);
    // make sure rtc temp boot mode doesn't persist
    rtc.next_mode = MODE_STANDARD;
    rtc.chksum = calc_chksum((uint8_t*)&rtc, (uint8_t*)&rtc.chksum);
    system_rtc_mem(RBOOT_RTC_ADDR, &rtc, sizeof(rboot_rtc_data), RBOOT_RTC_WRITE);
    return 0;
  }
#endif

  // check we have a good rom
  while (runAddr == 0) {
    ets_printf("Rom %d is bad.\r\n", romToBoot);
    // for normal mode try each previous rom
    // until we find a good one or run out
    updateConfig = TRUE;
    romToBoot--;
    if (romToBoot < 0) romToBoot = romconf->count - 1;
    if (romToBoot == romconf->current_rom) {
      // tried them all and all are bad!
      ets_printf("No good rom available.\r\n");
      return 0;
    }
    runAddr = check_image(romconf->roms[romToBoot]);
  }

  // re-write config, if required
  if (updateConfig) {
    romconf->current_rom = romToBoot;
#ifdef BOOT_CONFIG_CHKSUM
    romconf->chksum = calc_chksum((uint8_t*)romconf, (uint8_t*)&romconf->chksum);
#endif
    SPIEraseSector(BOOT_CONFIG_SECTOR);
    SPIWrite(BOOT_CONFIG_SECTOR * SECTOR_SIZE, buffer, SECTOR_SIZE);
  }

#ifdef BOOT_RTC_ENABLED
  // set rtc boot data for app to read
  rtc.magic = RBOOT_RTC_MAGIC;
  rtc.next_mode = MODE_STANDARD;
  rtc.last_mode = MODE_STANDARD;
  if (temp_boot) rtc.last_mode |= MODE_TEMP_ROM;
#ifdef BOOT_GPIO_ENABLED
  if (gpio_boot) rtc.last_mode |= MODE_GPIO_ROM;
#endif
  rtc.last_rom = romToBoot;
  rtc.temp_rom = 0;
  rtc.chksum = calc_chksum((uint8_t*)&rtc, (uint8_t*)&rtc.chksum);
  system_rtc_mem(RBOOT_RTC_ADDR, &rtc, sizeof(rboot_rtc_data), RBOOT_RTC_WRITE);
#endif

  ets_printf("Booting rom %d.\r\n", romToBoot);
  // copy the loader to top of iram
  ets_memcpy((void*)_text_addr, _text_data, _text_len);
  // return address to load from
  return runAddr;

}

#ifdef BOOT_NO_ASM

// small stub method to ensure minimum stack space used
void call_user_start(void) {
  uint32_t addr;
  stage2a *loader;

  addr = find_image();
  if (addr != 0) {
    loader = (stage2a*)entry_addr;
    loader(addr);
  }
}

#else

// assembler stub uses no stack space
// works with gcc
void call_user_start(void) {
  __asm volatile (
    "mov a15, a0\n"          // store return addr, hope nobody wanted a15!
    "call0 find_image\n"     // find a good rom to boot
    "mov a0, a15\n"          // restore return addr
    "bnez a2, 1f\n"          // ?success
    "ret\n"                  // no, return
    "1:\n"                   // yes...
    "movi a3, entry_addr\n"  // get pointer to entry_addr
    "l32i a3, a3, 0\n"       // get value of entry_addr
    "jx a3\n"                // now jump to it
  );
}

#endif
