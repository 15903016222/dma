class DmaData {
public:
        DmaData ();
	~DmaData ();
        void transmit_dma_data (void);
	void transmit_dma_data (int axis);
	void transmit_dma_data (int scanAxis, int stepAxis);
        void save_dma_data (void);
//        char *set_file_name (char *fileName);
        const char *get_file_name (void) {
            return m_fileName;
        }

	int m_scanDirection;
	int m_stepDirection;
        const char *m_fileName;
private:
	int m_fdMem;
	int m_fdFile;
	unsigned char *m_addr;
        unsigned int  *m_config;
	unsigned char *m_addrMem;
};
