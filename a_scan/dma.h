#ifndef __DMA_H__
#define __DMA_H__

#include <QMutex>

namespace DplSource {

class DmaPrivate;

class Dma
{
public:
    static Dma *instance();

    /**
     * @brief region_size   缓冲区大小
     * @return              大小(Bytes)
     */
    int region_size() const;

    /**
     * @brief read_data 读取缓冲区数据
     * @return          数据地址
     */
    const char *read_data();

    /**
     * @brief get_store_buffer  最终数据保存的内存区域
     * @return                  内存地址
     */
    const char *get_store_buffer();

    enum DrivingType {
        TIMER,
        ENCODER1,
        ENCODER2
    };

    /**
     * @brief driving_type  获取激励类型
     * @return              激励类型
     */
    unsigned int driving_type() const;

    /**
     * @brief set_driving_type  设置激励类型
     * @param type              类型
     */
    void set_driving_type(DrivingType type);

    /**
     * @brief frame_size    获取一帧数据的大小
     * @return              数据大小(Bytes)
     */
    int frame_size() const;

    /**
     * @brief frame_count   获取DMA一次传输多少帧数据
     * @return              数量
     */
    int frame_count() const;

    /**
     * @brief set_frame_count   设置DMA传输多少帧数据
     * @param value             数量
     */
    void set_frame_count(int count);

    /**
     * @brief encoder_offset    设置编码器偏移位置
     * @return                  偏移值
     */
    unsigned int encoder_offset() const;

    /**
     * @brief set_encoder_offset    设置编码器偏移位置
     * @param value
     */
    void set_encoder_offset(int value);

    /**
     * @brief steps_resolution  获取steps分辨率
     * @return                  分辨率
     */
    unsigned int steps_resolution() const;

    /**
     * @brief set_steps_resolution  设置steps分辨率
     * @param value                 分辨率
     */
    void set_steps_resolution(int value);

    /**
     * @brief start_offset  获取起点偏移位置
     * @return              偏移位置
     */
    unsigned int start_offset() const;

    /**
     * @brief set_start_offset  设置起点偏移位置
     * @param value             偏移位置
     */
    void set_start_offset(int value);

    unsigned int scan_timmer_counter() const;
    void set_scan_timmer_counter(int value);

    unsigned int get_scan_timmer_circled() const;
    void set_scan_timmer_circled(int value);

    unsigned char get_scan_data_mark(int index) const;

protected:
    Dma();
    virtual ~Dma();

private:
    DmaPrivate *d;
};

}

#endif // __DMA_H__
