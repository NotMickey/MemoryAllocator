//#include "pch.h"
#include "heapManager.h"

//#include <assert.h>
//#include <malloc.h>
//#include <stdlib.h>
#include <stdint.h>

#if _WIN64
#define BLOCK_SIZE 16
#elif _WIN32
#define BLOCK_SIZE 8
#endif

///////// Public functions /////////

MemoryAllocator::HeapManager * MemoryAllocator::HeapManager::create(void * i_pMemory, size_t i_sizeMemory,
																	unsigned int i_numDescriptors)
{
	return new HeapManager(i_pMemory, i_sizeMemory, i_numDescriptors);
}

void MemoryAllocator::HeapManager::destroy()
{
	//Call destroyers!
	destroyFreeList(h_freeList);
	h_freeList = nullptr;

	if (h_usedList != nullptr)
		destroyUsedList(h_usedList);

	h_usedList = nullptr;

	destroyDescriptorList(h_descriptorList);
	h_descriptorList = nullptr;

	startOfHeap = nullptr;
	endOfHeap = nullptr;
}

MemoryAllocator::HeapManager::~HeapManager()
{
	destroy();
}

size_t MemoryAllocator::HeapManager::getLargestFreeBlock() const
{
	BlockDescriptor *currBlock = h_freeList;
	size_t largestMemSize = 0;

	while (currBlock != nullptr) {

		if (currBlock->m_sizeBlock > largestMemSize) {

			largestMemSize = currBlock->m_sizeBlock;
		}

		currBlock = currBlock->nextBlock;
	}

	return largestMemSize;
}

size_t MemoryAllocator::HeapManager::getTotalFreeMemory() const
{
	BlockDescriptor *currBlock = h_freeList;
	size_t freeMemSize = 0;

	while (currBlock != nullptr) {

		freeMemSize += currBlock->m_sizeBlock;
		currBlock = currBlock->nextBlock;
	}

	return freeMemSize;
}



///////// Protected constructor /////////

MemoryAllocator::HeapManager::HeapManager(void * i_pMemory, size_t i_sizeMemory, unsigned int i_numDescriptors)
{
	//Init all lists
	h_freeList, t_freeList = nullptr;
	h_usedList, t_usedList = nullptr;
	h_descriptorList = nullptr;

	startOfHeap = static_cast<uint8_t*>(i_pMemory);
	endOfHeap = static_cast<uint8_t*>(i_pMemory) + i_sizeMemory;

	memorySize = i_sizeMemory;
	numDescriptors = i_numDescriptors;

	createDescriptorList(numDescriptors);
	createFreeList();
}



///////// Private functions /////////

void MemoryAllocator::HeapManager::createDescriptorList(size_t i_numDescriptors)
{
	h_descriptorList = (struct BlockDescriptor*)(static_cast<uint8_t*>(endOfHeap) - sizeof(struct BlockDescriptor)*i_numDescriptors);
	BlockDescriptor *descriptorBlockHead = nullptr;

	h_descriptorList->m_pBlockBase = static_cast<uint8_t*>(endOfHeap) - sizeof(struct BlockDescriptor)*i_numDescriptors;
	h_descriptorList->m_sizeBlock = sizeof(struct BlockDescriptor);
	h_descriptorList->inUse = false;
	h_descriptorList->m_pNext = nullptr;
	h_descriptorList->nextBlock = nullptr;

	descriptorBlockHead = h_descriptorList;

	for (size_t i = 1; i < i_numDescriptors; i++) {

		BlockDescriptor *descriptorBlock = (struct BlockDescriptor*)(static_cast<uint8_t*>(descriptorBlockHead->m_pBlockBase) + i * sizeof(struct BlockDescriptor));

		descriptorBlock->m_pBlockBase = static_cast<uint8_t*>(descriptorBlockHead->m_pBlockBase) + i * sizeof(struct BlockDescriptor);
		descriptorBlock->m_sizeBlock = sizeof(struct BlockDescriptor);
		descriptorBlock->inUse = false;
		descriptorBlock->m_pNext = nullptr;
		descriptorBlock->nextBlock = nullptr;

		h_descriptorList->m_pNext = descriptorBlock;
		h_descriptorList = descriptorBlock;
	}

	h_descriptorList = descriptorBlockHead;
}

void MemoryAllocator::HeapManager::createFreeList()
{
	h_descriptorList->inUse = true;
	h_descriptorList->m_pBlockBase = startOfHeap;
	h_descriptorList->m_sizeBlock = memorySize - numDescriptors * sizeof(struct BlockDescriptor);

	h_descriptorList->nextBlock = nullptr;

	h_freeList = h_descriptorList;
}

void MemoryAllocator::HeapManager::fillFreelistSorted(BlockDescriptor * i_block)
{
	if (h_freeList == nullptr)
	{
		h_freeList = i_block;
	}
	else if (static_cast<uint8_t*>(h_freeList->m_pBlockBase) >= static_cast<uint8_t*>(i_block->m_pBlockBase))
	{
		i_block->nextBlock = h_freeList;
		h_freeList = i_block;
	}
	else 
	{
		BlockDescriptor* currBlock = h_freeList;

		while (currBlock->nextBlock != nullptr && (static_cast<uint8_t*>(i_block->m_pBlockBase) > 
														static_cast<uint8_t*>(currBlock->nextBlock->m_pBlockBase)))
		{
			currBlock = currBlock->nextBlock;
		}

		i_block->nextBlock = currBlock->nextBlock;
		currBlock->nextBlock = i_block;
	}
}

MemoryAllocator::BlockDescriptor * MemoryAllocator::HeapManager::combineBlocks(BlockDescriptor * i_prevBlock,
																				BlockDescriptor * i_block)
{
	i_prevBlock->m_sizeBlock += i_block->m_sizeBlock;
	i_prevBlock->nextBlock = i_block->nextBlock;
	i_block->inUse = false;

	return i_prevBlock;
}

MemoryAllocator::BlockDescriptor * MemoryAllocator::HeapManager::getBlockDescriptor()
{
	BlockDescriptor* descriptor = h_descriptorList;

	while (descriptor->inUse == true && descriptor->m_pNext != nullptr) {

		descriptor = descriptor->m_pNext;
	}

	if (descriptor->inUse == true)
		return nullptr;

	descriptor->inUse = true;

	return descriptor;
}

void MemoryAllocator::HeapManager::destroyFreeList(BlockDescriptor *i_freeList)
{
	if (i_freeList != nullptr)
	{
		destroyFreeList(i_freeList->nextBlock);
		i_freeList = nullptr;
	}
}

void MemoryAllocator::HeapManager::destroyUsedList(BlockDescriptor *i_usedList)
{
	if (i_usedList != nullptr) 
	{
		destroyFreeList(i_usedList->nextBlock);
		i_usedList = nullptr;
	}
}

void MemoryAllocator::HeapManager::destroyDescriptorList(BlockDescriptor *i_descriptorList)
{
	if (i_descriptorList != nullptr) 
	{
		destroyFreeList(i_descriptorList->nextBlock);
		i_descriptorList = nullptr;
	}
}
