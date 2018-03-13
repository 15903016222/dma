#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "dma_data.h"

#define CONFIG_START_ADDR_OFFSET 0x00100000
#define STORE_BUFFER_START_ADDR  0x01000000

#define DATA_SAVE_BLOCK_SIZE_BIT     1024

#define DEV_NAME  "/dev/mem"
#define FILE_NAME "/media/card/data"

#define DmaFrameBuffer              m_config[0]
#define DataDmaCounter              m_config[1]
#define BufferInUse                 m_config[2]
#define X_ENCODER                   m_config[3]
#define StoreFrameCount             m_config[4]
#define X_ENCODER_COUNTER_OFFSET    m_config[5]
#define StepsPerResolution          m_config[6]
#define ScanZeroIndexOffset         m_config[7]
#define MaxStoreXIndex              m_config[8]
#define ScanTimmerCounter           m_config[9]
#define ScanTimmerCircled           m_config[10]
#define Y_ENCODER                   m_config[11]
#define Y_ENCODER_COUNTER_OFFSET    m_config[12]
#define Y_ENCODER_RESOLUTION        m_config[13]
#define Y_AREA_RESOLUTION           m_config[14]
#define Y_AREA_OFFSET               m_config[15]
#define MaxStoreYIndex              m_config[16]

// SaveDmaData 
DmaData::DmaData () : m_pos (0)
{
    m_fdMem  = open (DEV_NAME, O_RDWR);
    m_addr = (unsigned char *)mmap (NULL, 528 * 1024 * 1024,
                                    PROT_READ | PROT_WRITE, MAP_SHARED, m_fdMem, 0x2f000000);
    m_config = (unsigned int *)(m_addr + CONFIG_START_ADDR_OFFSET);
    m_addrMem = m_addr + STORE_BUFFER_START_ADDR;
}

DmaData::~DmaData ()
{
    if (m_fdMem < 0 || NULL == m_addr) {
        printf ("error \n");
    }
    munmap (m_addr, 528 * 1024 * 1024);
    close (m_fdMem);
}

void DmaData::transmit_dma_data (void)
{
    int res = -1;
    unsigned char *buff = NULL;
    unsigned int size = 0;
    unsigned int tmp_index = ScanTimmerCircled * MaxStoreXIndex + ScanTimmerCounter;
    for ( ; m_pos < tmp_index; ) {
        buff = (m_pos % MaxStoreXIndex) * StoreFrameCount * DATA_SAVE_BLOCK_SIZE_BIT + m_addrMem;
        if (m_pos / MaxStoreXIndex < ScanTimmerCircled) {
            size = (MaxStoreXIndex - m_pos % MaxStoreXIndex) * StoreFrameCount * DATA_SAVE_BLOCK_SIZE_BIT;
            m_pos = MaxStoreXIndex;
        } else {
            size = (tmp_index - m_pos) * StoreFrameCount * DATA_SAVE_BLOCK_SIZE_BIT;
        }
        res = write (m_fdFile, buff, size);
    }
    m_pos = tmp_index;
    printf ("%s[%d] \n", __func__, __LINE__);
    return ;
}

void DmaData::transmit_dma_data (int axis)
{

    printf ("%s[%d] \n", __func__, __LINE__);
    return ;
}

void DmaData::transmit_dma_data (int scanAxis, int stepAxis)
{

    printf ("%s[%d] \n", __func__, __LINE__);
    return ;
}

// name : NULL 使用系统默认的文件名字
//        其他  使用用户指定的文件名字
void DmaData::save_dma_data(void)
{
    printf ("%s[%d] \n", __func__, __LINE__);

    set_file_name(NULL);

    m_fdFile = open (m_fileName, O_RDWR | O_CREAT);
    if (m_fdFile < 0) {
        printf ("Open the file of saving dmadata failed. \n");
        return ;
    }

    if (X_ENCODER) {
        // ...
        if (Y_ENCODER) {
            // ...
        }
    } else {
        // scan timer
        transmit_dma_data();
    }

    return ;
}

char *DmaData::set_file_name (char *fileName)
{
    if (NULL == fileName) {
        if (m_fileName) {
            delete[] m_fileName;
            m_fileName = NULL;
        }
        m_fileName = new char[512];
        int i = 0;
        sprintf (m_fileName, "/media/card/data%d", i);
        while (!access (m_fileName, F_OK)) {
            sprintf (m_fileName, "/media/card/data%d", ++i);
        }
    } else {
        sprintf (m_fileName, "/media/card/%s", fileName);
    }

    printf ("%s\n", m_fileName);
    return m_fileName;
}

int m_scanDirection;
int m_stepDirection;
int m_fdMem;
int m_fdFile;
unsigned char *m_addr;
unsigned int *m_config;
unsigned char *m_addrMem;
const char *m_fileName;
