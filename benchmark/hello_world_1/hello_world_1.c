#include <stdio.h>
#include <stdint.h>

// Transfer Length in bytes
#define TRANSFER_LENGTH (8 * 16)	// Each Flit is of 8 Bytes so we send a total of 8 flits per packet

#define DMA_BUSY    1
#define DMA_ERR     2
#define DMA_DONE    3

uint64_t sendPacket[TRANSFER_LENGTH / 8];

#define SOURCE_X 1U
#define SOURCE_Y 1U

uint64_t volatile *NI_DMA_READ_ADDRESS = (uint64_t volatile *) 0x50000000;
uint64_t volatile *NI_DMA_READ_LENGTH = (uint64_t volatile *)  0x50000008;
uint64_t volatile *NI_DMA_READ_STATUS = (uint64_t volatile *)  0x50000010;
uint64_t volatile *NI_DMA_WRITE_ADDRESS = (uint64_t volatile *) 0x50000018;
uint64_t volatile *NI_DMA_WRITE_LENGTH = (uint64_t volatile *)  0x50000020;
uint64_t volatile *NI_DMA_WRITE_STATUS = (uint64_t volatile *)  0x50000028;

void __attribute__ ((noinline)) generateSendPacket(uint64_t *sendPacket, uint32_t dest_x, uint32_t dest_y, uint32_t messageNumber){
	int packetLength = TRANSFER_LENGTH / 8;
	uint64_t flit = 0;
	for(int i = 0; i < packetLength; i++){
		flit = 0;
		if(i == 0){
			flit |= 1lU << 62;

			flit |= messageNumber << 4;

			flit |= SOURCE_X << 3;
			flit |= SOURCE_Y << 2;

			flit |= dest_x << 1;
			flit |= dest_y;
		} else if(i == packetLength - 1){
			flit |= 3lU << 62;
		} else{
			flit |= 2lU << 62;
			flit |= messageNumber << 8;
			flit |= i;
		}

		sendPacket[i] = flit;
	}
}

void __attribute__ ((noinline)) sendToRouter(uint64_t *a, uint64_t transferLength){
	*NI_DMA_READ_ADDRESS = a;
	*NI_DMA_READ_LENGTH = transferLength;
}

void __attribute__ ((noinline)) receiveFromRouter(uint64_t *a, uint64_t transferLength){
	*NI_DMA_WRITE_ADDRESS = a;
	*NI_DMA_WRITE_LENGTH = transferLength;
}

// transferType is 1 for Read and 2 for Write
// Returns DMA_BUSY, DMA_ERROR and DMA_DONE
int __attribute__ ((noinline)) DMAStatus(int transferType){
	volatile uint64_t x;

	if(transferType == 1){
		x = *NI_DMA_READ_STATUS;
	} else{
		x = *NI_DMA_WRITE_STATUS;
	}

	return x;
}

int main() {

	uint64_t *a = (uint64_t *) 0x80010000U;       // Sender
	uint64_t *b = (uint64_t *) (0x80010000U + TRANSFER_LENGTH);   // Receiver
	
	for(int i = 0; i < TRANSFER_LENGTH / 8; i++){
		a[i] = 0;
		b[i] = 0;
	}

	generateSendPacket(a, 0, 0, 1);
	// For cache flush in RISC-V
	asm("FENCE");
	sendToRouter(a, TRANSFER_LENGTH);
	// printf("[Core 1]\tTest 1 Send Configured\n");
	// To check the status of the DMA
	while(DMAStatus(1) != DMA_DONE);
	// printf("[Core 1]\tTest 1 Sent\n");

	receiveFromRouter(b, TRANSFER_LENGTH);
	while(DMAStatus(2) != DMA_DONE);
	// For cache flush in RISC-V


	generateSendPacket(a, 0, 0, 2);
	asm("FENCE");
	sendToRouter(a, TRANSFER_LENGTH);
	// To check the status of the DMA
	while(DMAStatus(1) != DMA_DONE);

	receiveFromRouter(b, TRANSFER_LENGTH);
	while(DMAStatus(2) != DMA_DONE);
	// For cache flush in RISC-V


	generateSendPacket(a, 0, 0, 3);
	asm("FENCE");
	sendToRouter(a, TRANSFER_LENGTH);
	// To check the status of the DMA
	while(DMAStatus(1) != DMA_DONE);

	receiveFromRouter(b, TRANSFER_LENGTH);
	while(DMAStatus(2) != DMA_DONE);
	// For cache flush in RISC-V

	// uint64_t sum = 0;

	// for(int i = 0; i < TRANSFER_LENGTH / 8; i++){
	// 	sum += b[i];
	// }

	return 0;
}