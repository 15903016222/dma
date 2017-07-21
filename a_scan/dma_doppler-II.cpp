#include "dma.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include <QFile>
#include <QReadWriteLock>

namespace DplSource {

static const char *MEM_DEVICE               = "/dev/mem";
static const quint32 DATA_BUFFER_ADDR       = 0x2f000000;
static const int DATA_BUFFER_SIZE           = 16 * 1024 * 1024;     // 16M
static const quint32 STORE_BUFFER_ADDR      = 0x30000000;
static const int STORE_BUFFER_SIZE          = 512 * 1024 * 1024;    // 512M
static const int CONFIG_OFFSET              = 0x00400000;           // 4M
static const int SCAN_DATA_MARK_OFFSET      = 0x00500000;           // 5M
static const int REGION_SIZE                = 0x00100000;           // 1M
static const int DMA_DATA_OFFSET            = 2;
static const int FRAME_SIZE                 = 1024;

struct DmaParameter
{
    int hasData;                // DMA完成传输标志,驱动程序置位
    int counter;                // DMA传输次数
    int usedBufferFlag;         // 标志使用哪个缓冲区0～3

    int drivingType;            // 扫查源: 0:定时器； 1:编码器1； 2:编码器2
    int frameCount;             // DMA一次传输多少帧数据，一帧数据大小为1K（驱动设置为1K）
    int encoderOffset;          // 编码器在Beam中的偏移位置（Phascan只有X编码器）
    int stepResolution;         // 编码器分辩率
    int startOffset;            // 编码器起点

    int maxStoreQty;            // 最大保存数
    int scanTimmerCounter;      // 保存到storebuffer的次数
    int scanTimmerCircled;      // 定时器搜查源，保存循环次数（保存完整个storebuffer后，从头开始保存）
};


class DmaPrivate
{
public:
    DmaPrivate();
    ~DmaPrivate();

public:
    int m_fd;

    volatile DmaParameter *m_param;
    volatile quint8 *m_scanDataMark;

    char *m_dataBuffer;
    char *m_data[4];

    char *m_storeBuffer;

    QReadWriteLock m_rwlock;
};

DmaPrivate::DmaPrivate()
{
    if ((m_fd = ::open(MEM_DEVICE, O_RDWR | O_SYNC)) == -1) {
        qFatal("open memory device failed");
    }

    m_dataBuffer = (char *)::mmap (0,  DATA_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, DATA_BUFFER_ADDR);
    if (MAP_FAILED == m_dataBuffer) {
        qFatal("Mmap 0x%08x failed", DATA_BUFFER_ADDR);
    }
    for (int i = 0; i < 4; ++i) {
        m_data[i] = m_dataBuffer + DMA_DATA_OFFSET + REGION_SIZE*i;
    }

    m_storeBuffer = (char *)::mmap (0, STORE_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, STORE_BUFFER_ADDR);
    if (MAP_FAILED == m_storeBuffer) {
        qFatal("Mmap 0x%08x failed", STORE_BUFFER_ADDR);
    }

    m_param = (struct DmaParameter *)(m_dataBuffer + CONFIG_OFFSET);
    m_scanDataMark = (unsigned char *)(m_dataBuffer + SCAN_DATA_MARK_OFFSET);
}

DmaPrivate::~DmaPrivate()
{
    ::munmap((void *)m_dataBuffer,  DATA_BUFFER_SIZE);
    ::munmap((void *)m_storeBuffer, STORE_BUFFER_SIZE);
    ::close(m_fd);
}


/*** DMA ***/
int Dma::region_size() const
{
    return REGION_SIZE;
}

const char *Dma::read_data()
{
    QReadLocker l(&d->m_rwlock);

    if (d->m_param->hasData == 0) {
        return NULL;
    }
    d->m_param->hasData = 0;

    int i = (d->m_param->counter + 3) & 0x3;
    if (i == d->m_param->usedBufferFlag) {
        i = (i+3) & 0x3;
    }

    d->m_param->usedBufferFlag = i;

    return d->m_data[i];
}

const char *Dma::get_store_buffer()
{
    QReadLocker l(&d->m_rwlock);
    return d->m_storeBuffer;
}

unsigned int Dma::driving_type() const
{
	QReadLocker l(&d->m_rwlock);
    return d->m_param->drivingType;
}

void Dma::set_driving_type(DrivingType type)
{
	QWriteLocker l(&d->m_rwlock);
    qDebug("%s[%d]: drivingType(%d)",__func__, __LINE__, type);
    d->m_param->drivingType = type;
}

int Dma::frame_size() const
{
    return FRAME_SIZE;
}

int Dma::frame_count() const
{
    QReadLocker l(&d->m_rwlock);
    return d->m_param->frameCount;
}

void Dma::set_frame_count(int count)
{
    if (count <= 0) {
        return;
    }
	QWriteLocker l(&d->m_rwlock);
    d->m_param->frameCount = count;
    d->m_param->maxStoreQty = qMin( STORE_BUFFER_SIZE / (FRAME_SIZE*count), STORE_BUFFER_SIZE/FRAME_SIZE);

    qDebug("%s[%d]: framecount(%d) maxStoreQty(%d)",__func__,__LINE__,
           d->m_param->frameCount, d->m_param->maxStoreQty);
}

unsigned int Dma::encoder_offset() const
{
	QReadLocker l(&d->m_rwlock);
    return d->m_param->encoderOffset;
}

void Dma::set_encoder_offset(int value)
{
	QWriteLocker l(&d->m_rwlock);
    qDebug("%s[%d]: encoderOffset(%d)",__func__, __LINE__, value);
    d->m_param->encoderOffset = value;
}

unsigned int Dma::steps_resolution() const
{
	QReadLocker l(&d->m_rwlock);
    return d->m_param->stepResolution;
}
void Dma::set_steps_resolution(int value)
{
	QWriteLocker l(&d->m_rwlock);
    qDebug("%s[%d]: stepsResolution(%d)",__func__,__LINE__, value);
    d->m_param->stepResolution = value;
}

unsigned int Dma::start_offset() const
{
	QReadLocker l(&d->m_rwlock);
    return d->m_param->startOffset;
}

void Dma::set_start_offset(int value)
{
	QWriteLocker l(&d->m_rwlock);
    qDebug("%s[%d]: startOffset(%d)",__func__,__LINE__, value);
    d->m_param->startOffset = value;
}

unsigned int Dma::scan_timmer_counter() const
{
	QReadLocker l(&d->m_rwlock);
    return d->m_param->scanTimmerCounter;
}

void Dma::set_scan_timmer_counter(int value)
{
	QWriteLocker l(&d->m_rwlock);
    qDebug("%s[%d]: scanTimerCounter(%d)",__func__,__LINE__, value);
    d->m_param->scanTimmerCounter = value;
}

unsigned int Dma::get_scan_timmer_circled() const
{
	QReadLocker l(&d->m_rwlock);
    return d->m_param->scanTimmerCircled;
}

void Dma::set_scan_timmer_circled(int value)
{
	QWriteLocker l(&d->m_rwlock);
    qDebug("%s[%d]: scanTimerCircled(%d)",__func__,__LINE__, value);
    d->m_param->scanTimmerCircled = value;
}

unsigned char Dma::get_scan_data_mark(int index) const
{
    QReadLocker l(&d->m_rwlock);
    return d->m_scanDataMark[index];
}

Dma *Dma::instance()
{
    static Dma *ins = new Dma();
    return ins;
}

Dma::Dma()
    : d(new DmaPrivate())
{
}

Dma::~Dma()
{
    delete d;
}

}
