#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdbool.h>

#define MAX_CACHE_SIZE 8192

// ������ ���� ��Ʈ���� ����� ����
#define TRACE_FILE_NAME "ref_stream.txt"

// ���߿��Ḯ��Ʈ�� ��� Ÿ��
struct buffer
{
	unsigned long blkno;
	struct buffer* next, * prev;
};

// �����͸� ������ ������ �� ���� �Ҵ� ����
// ���� �߰����� �޸� �Ҵ��� ����
// �Ҵ� ���� ������ �̿��Ͽ� LRU ����Ʈ �Ǵ� FIFO ����Ʈ�� �����ؾ� ��
struct buffer cache_buffer[MAX_CACHE_SIZE];

// LRU �ùķ����� �� ��� lrulist �ƴϸ� fifolist�� �����ϱ� ���� ��� ��� ����
#if 0
struct buffer lrulist;
#else
struct buffer fifolist;
#endif

// int main(int argc, char* argv[])
int main(void)
{
	int ret = 0;
	unsigned long blkno;
	int idx = 0, hit = 0; // ���Ͽ��� �� �پ� �о���� ���� ����, hit ���� üũ�ϱ� ���� ����
	int fifo_idx = 0;		// fifo ��å�� ������ �� ĳ�� ���۸� �̵��ϱ� ���� ����
	FILE* fp = NULL;

	fifolist.next = NULL, fifolist.prev = NULL; // fifolist�� ������ �ʱ�ȭ

	// init();

	if ((fp = fopen(TRACE_FILE_NAME, "r")) == NULL)
	{
		printf("%s trace file open fail.\n", TRACE_FILE_NAME);

		return 0;
	}
	else
	{
		printf("start simulation!\n");
	}

	////////////////////////////////////////////////
	// �ùķ��̼� ���� �� ��� ����� ���� �ڵ�
	////////////////////////////////////////////////

	while (1)
	{
		fscanf(fp, "%d", &blkno); // �ؽ�Ʈ ���Ͽ��� �� �پ� ������ ����
		if (feof(fp))
			break; // ���������� �� �о��� ��� break

		bool isExist = false; // �Է¹��� ���� ���� ���� �̹� �����ϴ��� Ȯ���ϱ� ���� ����

		// ���ۿ� ���� ������ �������� ���
		if (cache_buffer[MAX_CACHE_SIZE - 1].blkno == 0)
		{
			// ���ۿ� ó�� �Է��ϴ� ��� �ٷ� ����
			if (idx == 0)
			{
				cache_buffer[idx].blkno = blkno;
				// ĳ�� ���ۿ��� ù �Է��̹Ƿ� fifolist�� prev, next�� ��� ù �����͸� ����Ŵ
				fifolist.prev = &cache_buffer[idx];
				fifolist.next = &cache_buffer[idx];
				idx++;
			}

			// ���۷��� �� ��° �Էº���
			else
			{
				// ĳ�� ���� ���� �����Ͱ� �ִ��� �˻�
				for (int i = 0; i < idx; i++)
				{
					// �����Ͱ� �̹� ���ۿ� �����ϸ� hit �ϰ� break
					if (blkno == cache_buffer[i].blkno)
					{
						hit++;
						isExist = true;
						break;
					}
				}
				// ĳ�� ���� ���� ���� �������� ���
				if (isExist == false)
				{
					cache_buffer[idx].blkno = blkno;
					// ĳ�� ���� �� prev, next�� ���� ������ ����Ű���� ��
					cache_buffer[idx].prev = &cache_buffer[idx - 1];
					if (idx != 8192) cache_buffer[idx].next = &cache_buffer[idx + 1];
					// �ֱٿ� ���� �������̹Ƿ� next�� ����Ű�� ��
					fifolist.next = &cache_buffer[idx];
					idx++;
				}
			}
		}

		// ���ۿ� ���� ������ ���� ���
		else
		{
			// ���� ĳ�� ���� ���� �����Ͱ� �ִ��� �˻�
			for (int i = 0; i < idx; i++)
			{
				// �����Ͱ� �̹� ���ۿ� �����ϸ� hit �ϰ� break
				if (blkno == cache_buffer[i].blkno)
				{
					hit++;
					isExist = true;
					break;
				}
			}
			// ĳ�� ���� ���� ���� �������� ���
			// ���⼭ FIFO ��å�� ���
			if (isExist == false)
			{
				if (fifolist.next != NULL && fifolist.prev != NULL)
				{
					fifolist.prev->blkno = blkno;                      // ���� ���� �����͸� ĳ�ù����� �� �տ� ����
					fifo_idx++;
					fifolist.prev = &cache_buffer[fifo_idx];			// fifolist�� prev�� ������ ������ ���� ������ ���� �����Ϳ� ��ġ��Ŵ
					if (fifo_idx == 8191) fifo_idx = 0;					// ĳ�ù����� ������ �迭 �����Ͱ� ��ü���� ��� �ٽ� ������ �̵�
				}
			}
		}
		ret++;
	}

	fclose(fp);

	float ratio = ((float)hit / (float)ret) * 100;

	printf("FIFO ��å�� HIT ratio�� %.4f �Դϴ�.\n", ratio);

	return 0;
}
