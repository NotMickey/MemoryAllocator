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
}

MemoryAllocator::HeapManager::~HeapManager()
{
	destroy();
}

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
