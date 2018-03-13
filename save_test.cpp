#include "dma_data.h"
#include <string.h>


int main (int argc, char *argv[]) 
{
	DmaData dmaData;
	dmaData.transmit_dma_data ();
	dmaData.transmit_dma_data (10);
	dmaData.transmit_dma_data (10, 20);
	dmaData.save_dma_data ();
	return 0;
}
