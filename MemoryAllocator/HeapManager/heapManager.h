#pragma once

namespace MemoryAllocator
{
	struct BlockDescriptor
	{

		void*  m_pBlockBase;
		size_t m_sizeBlock;
		bool inUse;

		BlockDescriptor* m_pNext;
		BlockDescriptor* nextBlock;
	};

	class HeapManager
	{
	
	public:
		// create a new HeapManager
		static HeapManager * create(void * i_pMemory, size_t i_sizeMemory, unsigned int i_numDescriptors);
		// destroy the HeapManager
		void	destroy();
		~HeapManager();

		// allocate - with and without alignment requirement
		void *	_alloc(size_t i_size);
		void *	_alloc(size_t i_size, unsigned int i_alignment);
		// free
		bool	_free(void * i_ptr);

		// run garbage collection
		void	collect();

		// query whether a given pointer is within this Heaps memory range
		bool	Contains(void * i_ptr) const;
		// query whether a given pointer is an outstanding allocation
		bool	IsAllocated(void * i_ptr) const;

		size_t	getLargestFreeBlock() const;
		size_t	getTotalFreeMemory() const;

		//class BlockAllocator *blockAllocator;

	protected:
		HeapManager(void * i_pMemory, size_t i_sizeMemory, unsigned int i_numDescriptors);

	private:

		void* endOfHeap;
		void* startOfHeap;
		size_t memorySize;
		unsigned int numDescriptors;

		void createDescriptorList(size_t i_numDescriptors);
		void createFreeList();

		BlockDescriptor *h_freeList, *t_freeList;
		BlockDescriptor *h_usedList, *t_usedList;
		BlockDescriptor *h_descriptorList;

		//void CreateFixedBlockAllocator();

		void fillFreelistSorted(BlockDescriptor* i_block);
		BlockDescriptor* combineBlocks(BlockDescriptor* i_prevBlock, BlockDescriptor* i_block);
		BlockDescriptor* getBlockDescriptor();

		void destroyFreeList(BlockDescriptor *i_freeList);
		void destroyUsedList(BlockDescriptor *i_usedList);
		void destroyDescriptorList(BlockDescriptor *i_descriptorList);
	};
}