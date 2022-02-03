#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdbool.h>

#define MAX_CACHE_SIZE 8192

// 데이터 참조 스트림이 저장된 파일
#define TRACE_FILE_NAME "ref_stream.txt"

// 이중연결리스트의 노드 타입
struct buffer
{
	unsigned long blkno;
	struct buffer* next, * prev;
};

// 데이터를 저장할 공간을 한 번에 할당 받음
// 이후 추가적인 메모리 할당은 없음
// 할당 받은 노드들을 이용하여 LRU 리스트 또는 FIFO 리스트를 유지해야 함
struct buffer cache_buffer[MAX_CACHE_SIZE];

// LRU 시뮬레이터 인 경우 lrulist 아니면 fifolist를 유지하기 위한 헤드 노드 선언
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
	int idx = 0, hit = 0; // 파일에서 한 줄씩 읽어오기 위한 변수, hit 수를 체크하기 위한 변수
	int fifo_idx = 0;		// fifo 정책을 시행할 때 캐시 버퍼를 이동하기 위한 변수
	FILE* fp = NULL;

	fifolist.next = NULL, fifolist.prev = NULL; // fifolist의 포인터 초기화

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
	// 시뮬레이션 시작 및 결과 출력을 위한 코드
	////////////////////////////////////////////////

	while (1)
	{
		fscanf(fp, "%d", &blkno); // 텍스트 파일에서 한 줄씩 정수를 읽음
		if (feof(fp))
			break; // 마지막까지 다 읽었을 경우 break

		bool isExist = false; // 입력받은 수가 버퍼 내에 이미 존재하는지 확인하기 위한 변수

		// 버퍼에 아직 공간이 남아있을 경우
		if (cache_buffer[MAX_CACHE_SIZE - 1].blkno == 0)
		{
			// 버퍼에 처음 입력하는 경우 바로 삽입
			if (idx == 0)
			{
				cache_buffer[idx].blkno = blkno;
				// 캐시 버퍼에의 첫 입력이므로 fifolist의 prev, next가 모두 첫 데이터를 가리킴
				fifolist.prev = &cache_buffer[idx];
				fifolist.next = &cache_buffer[idx];
				idx++;
			}

			// 버퍼로의 두 번째 입력부터
			else
			{
				// 캐시 버퍼 내에 데이터가 있는지 검사
				for (int i = 0; i < idx; i++)
				{
					// 데이터가 이미 버퍼에 존재하면 hit 하고 break
					if (blkno == cache_buffer[i].blkno)
					{
						hit++;
						isExist = true;
						break;
					}
				}
				// 캐시 버퍼 내에 없는 데이터일 경우
				if (isExist == false)
				{
					cache_buffer[idx].blkno = blkno;
					// 캐시 버퍼 내 prev, next가 각각 양쪽을 가리키도록 함
					cache_buffer[idx].prev = &cache_buffer[idx - 1];
					if (idx != 8192) cache_buffer[idx].next = &cache_buffer[idx + 1];
					// 최근에 들어온 데이터이므로 next가 가리키게 함
					fifolist.next = &cache_buffer[idx];
					idx++;
				}
			}
		}

		// 버퍼에 남은 공간이 없을 경우
		else
		{
			// 먼저 캐시 버퍼 내에 데이터가 있는지 검사
			for (int i = 0; i < idx; i++)
			{
				// 데이터가 이미 버퍼에 존재하면 hit 하고 break
				if (blkno == cache_buffer[i].blkno)
				{
					hit++;
					isExist = true;
					break;
				}
			}
			// 캐시 버퍼 내에 없는 데이터일 경우
			// 여기서 FIFO 정책을 사용
			if (isExist == false)
			{
				if (fifolist.next != NULL && fifolist.prev != NULL)
				{
					fifolist.prev->blkno = blkno;                      // 새로 들어온 데이터를 캐시버퍼의 맨 앞에 삽입
					fifo_idx++;
					fifolist.prev = &cache_buffer[fifo_idx];			// fifolist의 prev를 삭제된 데이터 다음 순서로 들어온 데이터에 위치시킴
					if (fifo_idx == 8191) fifo_idx = 0;					// 캐시버퍼의 마지막 배열 데이터가 교체됐을 경우 다시 앞으로 이동
				}
			}
		}
		ret++;
	}

	fclose(fp);

	float ratio = ((float)hit / (float)ret) * 100;

	printf("FIFO 정책의 HIT ratio는 %.4f 입니다.\n", ratio);

	return 0;
}
