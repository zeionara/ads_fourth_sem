#include <stdio.h>
#include <string.h>
#include <fstream>
#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <time.h>
using namespace std;

#define HEAP_SIZE_DEFAULT 0x8000

typedef struct _PriorityQueueRecord{
  DWORD name;
  DWORD key;
} PriorityQueueRecord, *PriorityQueueRecordPointer;

typedef struct _GraphAdjRecord GraphAdjRecord, *GraphAdjRecordPointer;

typedef struct _GraphAdjRecord{
  DWORD name;
  DWORD weight;
  GraphAdjRecordPointer next;
} GraphAdjRecord, *GraphAdjRecordPointer;

//
//
//Graph Generating Methods
//
//

bool wasTheConnection(GraphAdjRecordPointer graphAdjRecord, DWORD vertexName){
  while (graphAdjRecord != NULL){
    if (graphAdjRecord->name == vertexName) return true;
    graphAdjRecord = graphAdjRecord->next;
  }
  return false;
}

GraphAdjRecordPointer generateGraphAdj(HANDLE hGraphAdjHeap, DWORD graphSize, DWORD graphDensity, DWORD minWeight, DWORD maxWeight){
  GraphAdjRecordPointer graphAdj = (GraphAdjRecordPointer)HeapAlloc(hGraphAdjHeap, HEAP_ZERO_MEMORY, graphSize*sizeof(GraphAdjRecord));
  GraphAdjRecordPointer currentAdjRecord = graphAdj;
  DWORD vertexName = 0;
  srand(time(NULL));
  for (DWORD i = 0; i < graphSize; i++){  // for each v
    currentAdjRecord = &graphAdj[i];
    currentAdjRecord->next = NULL;
    vertexName = i;
    while ((vertexName == i) || (wasTheConnection(&graphAdj[i],vertexName))) vertexName = rand() % graphSize;
    currentAdjRecord->weight = rand() % (maxWeight - minWeight) + minWeight;
    currentAdjRecord->name = vertexName;
    for (DWORD j = 0; j < graphDensity - 1; j++){
      currentAdjRecord->next = (GraphAdjRecordPointer)HeapAlloc(hGraphAdjHeap, HEAP_ZERO_MEMORY, sizeof(GraphAdjRecord));
      currentAdjRecord = currentAdjRecord->next;
      vertexName = i;
      while ((vertexName == i) || (wasTheConnection(&graphAdj[i],vertexName))) vertexName = rand() % graphSize;
      currentAdjRecord->weight = rand() % (maxWeight - minWeight) + minWeight;
      currentAdjRecord->name = vertexName;
      currentAdjRecord->next = NULL;
    }
  }
  return graphAdj;
}

int showGraph(GraphAdjRecordPointer graphAdj, DWORD graphSize, DWORD graphDensity){
  GraphAdjRecordPointer currentAdjRecord = graphAdj;
  for (int i = 0; i < graphSize; i++){
    currentAdjRecord = &graphAdj[i];
    cout << "Vertex " << i << " : " << endl;
    for (int i = 0; i < graphDensity; i++){
      cout << "  ";
      cout << currentAdjRecord->name << " with weight " << currentAdjRecord->weight << endl;
      currentAdjRecord = currentAdjRecord->next;
    }
  }
  return 0;
}

//
//
//Priority Queue Methods
//
//

int showChilds(PriorityQueueRecordPointer priorityQueue, DWORD recordIndex, DWORD queueLength, int queueWidth){
  if (recordIndex >= queueLength) return 0;
  DWORD numOfSpaces = recordIndex;
  while (numOfSpaces > 0){
    cout << "        ";
    numOfSpaces = (numOfSpaces-1)/queueWidth;
  }
  cout << "Name = " << priorityQueue[recordIndex].name << " Key = " << priorityQueue[recordIndex].key << endl;
  DWORD childIndex = recordIndex*queueWidth + 1;
  DWORD childMaxIndex = (recordIndex+1)*queueWidth;
  for (DWORD childIndex = recordIndex*queueWidth + 1; childIndex <= (recordIndex+1)*queueWidth; childIndex++)
    showChilds(priorityQueue, childIndex, queueLength, queueWidth);
  return 0;
}

int showPriorityQueue(PriorityQueueRecordPointer priorityQueue,DWORD queueLength,int queueWidth){
  showChilds(priorityQueue, 0, queueLength, queueWidth);
  return 0;
}

PriorityQueueRecordPointer generatePriorityQueue(HANDLE hPriorityQueueHeap, DWORD queueLength, int minName, int maxName, int minKey, int maxKey){
  PriorityQueueRecordPointer priorityQueue = (PriorityQueueRecordPointer)HeapAlloc(hPriorityQueueHeap, HEAP_ZERO_MEMORY, queueLength*sizeof(PriorityQueueRecord));
  srand(time(NULL));
  for (int i = 0; i < queueLength; i++){
    priorityQueue[i].name = rand() % (maxName - minName) + minName;
    priorityQueue[i].key = rand() % (maxKey - minKey) + minKey;
  }
  return priorityQueue;
}

DWORD firstChildIndex(DWORD queueLength, int queueWidth, DWORD recordIndex){
  DWORD firstChildIndex = recordIndex*queueWidth + 1;
  if (firstChildIndex > queueLength) return queueLength;
  return firstChildIndex;
}

DWORD lastChildIndex(DWORD queueLength, int queueWidth, DWORD recordIndex){
  DWORD firstChildId = firstChildIndex(queueLength, queueWidth, recordIndex);
  if (firstChildId == queueLength) return queueLength;
  DWORD lastChildIndex = firstChildId + queueWidth - 1;
  if (lastChildIndex > queueLength) return queueLength;
  return lastChildIndex;
}

DWORD minChildIndex(DWORD queueLength, int queueWidth, DWORD recordIndex, PriorityQueueRecordPointer priorityQueue){ // asymptotically O(d)
  DWORD firstChildId = firstChildIndex(queueLength, queueWidth, recordIndex);
  if (firstChildId == queueLength) return queueLength;
  DWORD lastChildId = lastChildIndex(queueLength, queueWidth, recordIndex);
  DWORD minChildId = firstChildId;
  for (int i = firstChildId+1; i <= lastChildId; i++){  // gives O(d)
    if (priorityQueue[minChildId].key > priorityQueue[i].key){
      minChildId = i;
    }
  }
  return minChildId;
}

DWORD parentIndex(DWORD queueLength, int queueWidth, DWORD recordIndex){
  return (recordIndex - 1)/ queueWidth;
}

int moveDown(DWORD queueLength, int queueWidth, DWORD recordIndex, PriorityQueueRecordPointer priorityQueue, LPDWORD indexes){// asymptotically O(dlog(n))
  DWORD recordKey = priorityQueue[recordIndex].key;
  DWORD recordName = priorityQueue[recordIndex].name;
  DWORD minChildId = minChildIndex(queueLength, queueWidth, recordIndex, priorityQueue);
  while ((minChildId != queueLength) && (recordKey > priorityQueue[minChildId].key)){ // gives log(n)
      priorityQueue[recordIndex].key = priorityQueue[minChildId].key;
      priorityQueue[recordIndex].name = priorityQueue[minChildId].name;
      indexes[priorityQueue[recordIndex].name] = recordIndex;
      recordIndex = minChildId;
      minChildId = minChildIndex(queueLength, queueWidth, recordIndex, priorityQueue); //gives d
  }
  priorityQueue[recordIndex].key = recordKey;
  priorityQueue[recordIndex].name = recordName;
  indexes[priorityQueue[recordIndex].name] = recordIndex;
  return 0;
}

int moveUp(DWORD queueLength, int queueWidth, DWORD recordIndex, PriorityQueueRecordPointer priorityQueue, LPDWORD indexes){// asymptotically O(log(n))
  DWORD recordKey = priorityQueue[recordIndex].key;
  DWORD recordName = priorityQueue[recordIndex].name;
  DWORD parentId = parentIndex(queueLength, queueWidth, recordIndex);
  while ((parentId != 0) && (priorityQueue[parentId].key > recordKey)){ // gives log(n)
    priorityQueue[recordIndex].key = priorityQueue[parentId].key;
    priorityQueue[recordIndex].name = priorityQueue[parentId].name;
    indexes[priorityQueue[recordIndex].name] = recordIndex;
    recordIndex = parentId;
    parentId = parentIndex(queueLength, queueWidth, recordIndex);
  }
  priorityQueue[parentId].key = recordKey;
  priorityQueue[parentId].name = recordName;
  indexes[priorityQueue[recordIndex].name] = recordIndex;
  return 0;
}

PriorityQueueRecord takeMininum(DWORD queueLength, int queueWidth, DWORD recordIndex, PriorityQueueRecordPointer priorityQueue, LPDWORD indexes){ // asymptotically O(dlog(n))
  PriorityQueueRecord minRecord;
  minRecord.key = priorityQueue[0].key;
  minRecord.name = priorityQueue[0].name;
  priorityQueue[0].key = priorityQueue[queueLength - 1].key;
  priorityQueue[0].name = priorityQueue[queueLength - 1].name;
  if (queueLength - 1 > 1) moveDown(queueLength - 1, queueWidth, 0, priorityQueue, indexes); // gives O(dlog(n))
  return minRecord;
}

int makePriorityQueue(DWORD queueLength, int queueWidth, PriorityQueueRecordPointer priorityQueue, LPDWORD indexes){// asymptotically O(n)
  for (int recordIndex = queueLength - 1; recordIndex >= 0; recordIndex--){// gives O(n)
    moveDown(queueLength, queueWidth, recordIndex, priorityQueue, indexes);// gives O(dlog(n))
  }
  return 0;
}

//
//
//Dijkstra algorythm methods
//
//

//
//
// Common methods
//
//

int main(int argc, char* argv[]){
  int queueWidth = 4;
  DWORD queueLength = 100;
  DWORD graphSize = 1000;
  DWORD graphDensity = 10;

  HANDLE hPriorityQueueHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS | HEAP_NO_SERIALIZE, queueLength*sizeof(PriorityQueueRecord)+1, 0);
  HANDLE hGraphAdjHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS | HEAP_NO_SERIALIZE, graphSize*graphDensity*sizeof(GraphAdjRecord)+1, 0);

  GraphAdjRecordPointer graphAdj = generateGraphAdj(hGraphAdjHeap, graphSize, graphDensity, 100, 200);
  //showGraph(graphAdj, graphSize, graphDensity);

  PriorityQueueRecordPointer priorityQueue = generatePriorityQueue(hPriorityQueueHeap, queueLength, 0, 100, 1000, 2000);
  //showPriorityQueue(priorityQueue,queueLength,queueWidth);
  cout << firstChildIndex(100,4,2) << endl;
  cout << parentIndex(100, 4, firstChildIndex(100,4,2)) << endl;
  cout << parentIndex(100, 4, lastChildIndex(100,4,2)) << endl;
  cout << lastChildIndex(100, 4, 2) << endl;
  cout << minChildIndex(100, 4, 2, priorityQueue) << endl;

  LPDWORD indexes = (LPDWORD)malloc(queueLength*sizeof(DWORD));
  makePriorityQueue(100,4,priorityQueue, indexes);
  //showPriorityQueue(priorityQueue,queueLength,queueWidth);
}
