/*
 * Authors (alphabetical order)
 * - Bertrand Songis <bsongis@gmail.com>
 * - Bryan J. Rentoul (Gruvin) <gruvin@gmail.com>
 * - Cameron Weeks <th9xer@gmail.com>
 * - Erez Raviv
 * - Jean-Pierre Parisy
 * - Karl Szmutny <shadow@privy.de>
 * - Michael Blandford
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini <romolo.manfredini@gmail.com>
 * - Thomas Husterer
 *
 * open9x is based on code named
 * gruvin9x by Bryan J. Rentoul: http://code.google.com/p/gruvin9x/,
 * er9x by Erez Raviv: http://code.google.com/p/er9x/,
 * and the original (and ongoing) project by
 * Thomas Husterer, th9x: http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "ersky9x.h"
#include "file.h"
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

volatile uint8_t pinb=0xff, pinc=0xff, pind, pine=0xff, ping=0xff, pinh=0xff, pinj=0xff, pinl=0;
uint8_t portb, portc, porth=0, dummyport;
uint16_t dummyport16;
const char *eepromFile = NULL;
FILE *fp = NULL;

Pio Pioa, Piob, Pioc;
Twi Twio;
Usart Usart0;
Pwm pwm;
uint32_t eeprom_pointer;
char* eeprom_buffer_data;
volatile int32_t eeprom_buffer_size;
bool eeprom_read_operation;
#define EESIZE (128*4096)
void configure_pins( uint32_t pins, uint16_t config ) { }

uint8_t eeprom[EESIZE];
sem_t *eeprom_write_sem;

void setSwitch(int8_t swtch)
{
  switch (swtch) {
    case DSW_ID0:
      PIOC->PIO_PDSR &= ~0x00004000;
      PIOC->PIO_PDSR |= 0x00000800;
      break;
    case DSW_ID1:
      PIOC->PIO_PDSR |= 0x00004800;
      break;
    case DSW_ID2:
      PIOC->PIO_PDSR &= ~0x00000800;
      PIOC->PIO_PDSR |= 0x00004000;
      break;
    default:
      break;
  }
}

bool eeprom_thread_running = true;
void *eeprom_write_function(void *)
{
  while (!sem_wait(eeprom_write_sem)) {
    if (!eeprom_thread_running)
      return NULL;
    if (eeprom_read_operation) {
      assert(eeprom_buffer_size);
      eeprom_read_block(eeprom_buffer_data, (const void *)(int64_t)eeprom_pointer, eeprom_buffer_size);
      // TODO sleep()
    }
    else {
    if (fp) {
      if (fseek(fp, eeprom_pointer, SEEK_SET) == -1)
        perror("error in fseek");
    }
    while (--eeprom_buffer_size) {
      assert(eeprom_buffer_size > 0);
      if (fp) {
        if (fwrite(eeprom_buffer_data, 1, 1, fp) != 1)
          perror("error in fwrite");
      }
      else {
        memcpy(&eeprom[eeprom_pointer], eeprom_buffer_data, 1);
      }
      eeprom_pointer++;
      eeprom_buffer_data++;
      
      if (fp && eeprom_buffer_size == 1) {
        fflush(fp);
      }
    }
    }
    Spi_complete = 1;
  }
  return 0;
}

uint8_t main_thread_running = 0;
char * main_thread_error = NULL;
void *main_thread(void *)
{
#ifdef SIMU_EXCEPTIONS
  signal(SIGFPE, sig);
  signal(SIGSEGV, sig);

  try {
#endif

    // TODO s_current_protocol = 255;

    g_menuStackPtr = 0;
    g_menuStack[0] = menuProc0;
    g_menuStack[1] = menuProcModelSelect;

    init_eeprom();

    eeReadAll(); //load general setup and selected model

    if (main_thread_running == 1) {
      doSplash();
      checkTHR();
      checkSwitches();
      // TODO not in ersky9x? checkAlarm();
    }

    // TODO s_current_protocol = 0;

    while (main_thread_running) {
      perMain(0);
      sleep(1/*ms*/);
    }
#ifdef SIMU_EXCEPTIONS
  }
  catch (...) {
    main_thread_running = 0;
  }
#endif

  return NULL;
}

pthread_t main_thread_pid;
void StartMainThread(bool tests)
{
  main_thread_running = (tests ? 1 : 2);
  pthread_create(&main_thread_pid, NULL, &main_thread, NULL);
}

void StopMainThread()
{
  main_thread_running = 0;
  pthread_join(main_thread_pid, NULL);
}

pthread_t eeprom_thread_pid;
void StartEepromThread(const char *filename)
{
  eepromFile = filename;
  if (eepromFile) {
    fp = fopen(eepromFile, "r+");
    if (!fp)
      fp = fopen(eepromFile, "w+");
    if (!fp) perror("error in fopen");
  }
#ifdef __APPLE__
  eeprom_write_sem = sem_open("eepromsemaphore", O_CREAT);
#else
  eeprom_write_sem = (sem_t *)malloc(sizeof(sem_t));
  sem_init(eeprom_write_sem, 0, 0);
#endif
  eeprom_thread_running = true;
  assert(!pthread_create(&eeprom_thread_pid, NULL, &eeprom_write_function, NULL));
}

void StopEepromThread()
{
  eeprom_thread_running = false;
  sem_post(eeprom_write_sem);
  pthread_join(eeprom_thread_pid, NULL);
}

void eeprom_read_block (void *pointer_ram,
    const void *pointer_eeprom,
    uint32_t size)
{
  assert(size);

  if (fp) {
    memset(pointer_ram, 0, size);
    if (fseek(fp, (long) pointer_eeprom, SEEK_SET)==-1) perror("error in seek");
    if (fread(pointer_ram, size, 1, fp) <= 0) perror("error in read");
  }
  else {
    memcpy(pointer_ram, &eeprom[(uint64_t)pointer_eeprom], size);
  }
}

uint16_t stack_free()
{
  return 500;
}

void init_main_ppm( uint32_t period, uint32_t out_enable )
{
}

void disable_main_ppm()
{
}

uint8_t pxxFlag = 0 ;
