# 9.4 Ext2的分配策略

当建立一个新文件或目录时，Ext2 必须决定在磁盘上的什么地方存储数据，也就是说， 将哪些物理块分配给这个新文件或目录。一个好的分配物理块的策略，将导致文件系统性能的提高。一个好的思路是将相关的数据尽量存储在磁盘上相邻的区域，以减少磁头的寻道时间。Ext2 使用块组的优越性就体现出来了，因为，同一个组中的逻辑块所对应的物理块通常是相邻存储的。Ext2 企图将每一个新的目录分到它的父目录所在的组，因为在理论上，访问完父目录后，接着要访问其子目录，例如对一个路径的解析。所以，将父目录和子目录放在
同一个组是有必要的。它还企图将文件和它的目录项分在同一个组，因为目录访问常常导致
文件访问。当然如果组已满，则文件或目录可能分在某一个未满的组中。

分配空间的大致算法如下：

> 1. 文件的数据块尽量和它的索引节点在同一个组中
> 2. 每个文件的数据块尽量连续分配
> 3. 父目录和子目录尽量在一个块组
> 4. 文件和它的目录项尽量在同一个块组中

### 9.4.1 数据块寻址

每个非空的普通文件都是由一组数据块组成。这些块或者由文件内的**相对位置(文件块**
**号)**来表示，或者由**磁盘分区内的位置(它们的逻辑块号)**来表示。从文件内的偏移量 f 导出相应数据块的逻辑块号需要以下两个步骤, 因为 Linux 文件不包含任何控制字符，因此，导出文件的第 f 个字符所在的文件块号是相当容易的，只是用 f 除以文件系统块的大小，并取整即可： 

* 从偏移量 f 导出文件的块号，即偏移量 f 处的字符所在的块索引。
* 把文件的块号转化为相应的逻辑块号。

由于Ext2 文件的数据块在磁盘上并不是物理相邻的，所以把文件的块号转化为相应的逻辑块号可能效率不高，所以Ext2 文件系统提供了一种方法，用这种方法可以在磁盘上建立每个文件块号与 相应逻辑块号之间的关系。在索引节点内部部分实现了这种映射，这种映射也包括一些专门的数据块，可以把这些数据块看成是用来处理大型文件的索引节点的扩展。

磁盘索引节点的 `i_block` 域是一个有 `EXT2_N_BLOCKS` 个元素且包含逻辑块号的数组。在 下面的讨论中，我们假定` EXT2_N_BLOCKS` 的默认值为 15，这个数组表示一个大型数据结构的初始化部分，数组的 15 个元素有 4 种不同的类型：

* 最初的12个元素产生的逻辑块号与文件最初的12个块对应，即对应的文件块号从0 到11
* 索引 12 中的元素包含一个块的逻辑块号，这个块代表逻辑块号的一个二级数组。这 个数组对应的文件块号从 12 到 b/4+11，这里 b 是文件系统的块大小(每个逻辑块号占 4 个字节，因此我们在式子中用 4 做除数)。因此，内核必须先用指向一个块的指针访问这个元素， 然后，用另一个指向包含文件最终内容的块的指针访问那个块。
* 索引 13 中的元素包含一个块的逻辑块号，这个块包含逻辑块号的一个二级数组;这个二级数组的数组项依次指向三级数组，这个三级数组存放的才是逻辑块号对应的文件块号， 范围从 b/4+12 到(b/4)2+(b/4)+11
* 最后，索引 14 中的元素利用了三级间接索引:第四级数组中存放的才是逻辑块号对应的文件块号，范围从(b/4)2+(b/4)+12 到(b/4)3+(b/4)2+(b/4)+11

如果文件需要的数据块小于 12，那么两次访问磁盘就可以检索到任何数据:一次是读磁盘索引节点 i_block 数组的一个元素，另一次是读所需要 的数据块。对于大文件来说，可能需要 3~4 次的磁盘访问才能找到需要的块。实际上，这是 一种最坏的估计，因为目录项、缓冲区及页高速缓存都有助于极大地减少实际访问磁盘的次数。

![3_1](./images/3_1.png)

### 9.4.2 分配一个新的数据块

当内核要分配一个新的数据块来保存 Ext2 普通文件的数据时，就调用`ext2_get_block( )`函数。这个函数依次处理在“数据块寻址”部分所描述的那些数据结构， 并在必要时调用`ext2_alloc_block()`函数在Ext2分区实际搜索一个空闲的块.

为了减少文件的碎片，Ext2 文件系统尽力在已分配给文件的最后一个块附近找一个新块 分配给该文件。如果失败，Ext2 文件系统又在包含这个文件索引节点的块组中搜寻一个新的 块。作为最后一个办法，可以从其他一个块组中获得空闲块。

Ext2 文件系统使用数据块的预分配策略。文件并不仅仅获得所需要的块，而是获得一组 多达8个邻接的块。ext2_inode_info结构的i_prealloc_count域存放预分配给某一文件但 还没有使用的数据块数，而 i_prealloc_block 域存放下一次要使用的预分配块的逻辑块号。 当下列情况发生时，即文件被关闭时，文件被删除时，或关于引发块预分配的写操作而言， 有一个写操作不是顺序的时候，就释放预分配但一直没有使用的块。`ext2_get_block()`如下：

```c
static int ext2_get_blocks(struct inode *inode,
			   sector_t iblock, unsigned long maxblocks,
			   u32 *bno, bool *new, bool *boundary,
			   int create)
{
	int err;
	int offsets[4];
	Indirect chain[4];
	Indirect *partial;
	ext2_fsblk_t goal;
	int indirect_blks;
	int blocks_to_boundary = 0;
	int depth;
	struct ext2_inode_info *ei = EXT2_I(inode);
	int count = 0;
	ext2_fsblk_t first_block = 0;

	BUG_ON(maxblocks == 0);

	depth = ext2_block_to_path(inode,iblock,offsets,&blocks_to_boundary);

	if (depth == 0)
		return -EIO;

	partial = ext2_get_branch(inode, depth, offsets, chain, &err);
	/* Simplest case - block found, no allocation needed */
	if (!partial) {
		first_block = le32_to_cpu(chain[depth - 1].key);
		count++;
		/*map more blocks*/
		while (count < maxblocks && count <= blocks_to_boundary) {
			ext2_fsblk_t blk;

			if (!verify_chain(chain, chain + depth - 1)) {
				/*
				 * Indirect block might be removed by
				 * truncate while we were reading it.
				 * Handling of that case: forget what we've
				 * got now, go to reread.
				 */
				err = -EAGAIN;
				count = 0;
				partial = chain + depth - 1;
				break;
			}
			blk = le32_to_cpu(*(chain[depth-1].p + count));
			if (blk == first_block + count)
				count++;
			else
				break;
		}
		if (err != -EAGAIN)
			goto got_it;
	}

	/* Next simple case - plain lookup or failed read of indirect block */
	if (!create || err == -EIO)
		goto cleanup;

	mutex_lock(&ei->truncate_mutex);
	/*
	 * If the indirect block is missing while we are reading
	 * the chain(ext2_get_branch() returns -EAGAIN err), or
	 * if the chain has been changed after we grab the semaphore,
	 * (either because another process truncated this branch, or
	 * another get_block allocated this branch) re-grab the chain to see if
	 * the request block has been allocated or not.
	 *
	 * Since we already block the truncate/other get_block
	 * at this point, we will have the current copy of the chain when we
	 * splice the branch into the tree.
	 */
	if (err == -EAGAIN || !verify_chain(chain, partial)) {
		while (partial > chain) {
			brelse(partial->bh);
			partial--;
		}
		partial = ext2_get_branch(inode, depth, offsets, chain, &err);
		if (!partial) {
			count++;
			mutex_unlock(&ei->truncate_mutex);
			goto got_it;
		}

		if (err) {
			mutex_unlock(&ei->truncate_mutex);
			goto cleanup;
		}
	}

	/*
	 * Okay, we need to do block allocation.  Lazily initialize the block
	 * allocation info here if necessary
	*/
	if (S_ISREG(inode->i_mode) && (!ei->i_block_alloc_info))
		ext2_init_block_alloc_info(inode);

	goal = ext2_find_goal(inode, iblock, partial);

	/* the number of blocks need to allocate for [d,t]indirect blocks */
	indirect_blks = (chain + depth) - partial - 1;
	/*
	 * Next look up the indirect map to count the total number of
	 * direct blocks to allocate for this branch.
	 */
	count = ext2_blks_to_allocate(partial, indirect_blks,
					maxblocks, blocks_to_boundary);
	/*
	 * XXX ???? Block out ext2_truncate while we alter the tree
	 */
	err = ext2_alloc_branch(inode, indirect_blks, &count, goal,
				offsets + (partial - chain), partial);

	if (err) {
		mutex_unlock(&ei->truncate_mutex);
		goto cleanup;
	}

	if (IS_DAX(inode)) {
		/*
		 * We must unmap blocks before zeroing so that writeback cannot
		 * overwrite zeros with stale data from block device page cache.
		 */
		clean_bdev_aliases(inode->i_sb->s_bdev,
				   le32_to_cpu(chain[depth-1].key),
				   count);
		/*
		 * block must be initialised before we put it in the tree
		 * so that it's not found by another thread before it's
		 * initialised
		 */
		err = sb_issue_zeroout(inode->i_sb,
				le32_to_cpu(chain[depth-1].key), count,
				GFP_NOFS);
		if (err) {
			mutex_unlock(&ei->truncate_mutex);
			goto cleanup;
		}
	}
	*new = true;

	ext2_splice_branch(inode, iblock, partial, indirect_blks, count);
	mutex_unlock(&ei->truncate_mutex);
got_it:
	if (count > blocks_to_boundary)
		*boundary = true;
	err = count;
	/* Clean up and exit */
	partial = chain + depth - 1;	/* the whole chain */
cleanup:
	while (partial > chain) {
		brelse(partial->bh);
		partial--;
	}
	if (err > 0)
		*bno = le32_to_cpu(chain[depth-1].key);
	return err;
}

int ext2_get_block(struct inode *inode, sector_t iblock,
		struct buffer_head *bh_result, int create)
```

