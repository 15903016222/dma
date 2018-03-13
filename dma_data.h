class DmaData {
public:
        DmaData ();
	~DmaData ();
        void transmit_dma_data (void);
	void transmit_dma_data (int axis);
	void transmit_dma_data (int scanAxis, int stepAxis);
        void save_dma_data (void);
        char *set_file_name (char *fileName);
        char *get_file_name (void) {
            return m_fileName;
        }

	int m_scanDirection;
	int m_stepDirection;
        char *m_fileName;
private:
	int m_fdMem;
	int m_fdFile;
        unsigned int  m_pos;
	unsigned char *m_addr;
        unsigned int  *m_config;
	unsigned char *m_addrMem;
};
