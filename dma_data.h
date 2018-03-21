class DmaData {
public:
        DmaData ();
	~DmaData ();
        void transmit_dma_data (void);
	void transmit_dma_data (int axis);
	void transmit_dma_data (int scanAxis, int stepAxis);
        void save_dma_data (void);
        const char *get_file_name (void) {
            return m_fileName;
        }

private:
	int m_fdMem;
	int m_fdFile;
        const char *m_fileName;
	unsigned char *m_addr;
        unsigned int  *m_config;
	unsigned char *m_addrMem;
        unsigned int m_current_pos;
};
