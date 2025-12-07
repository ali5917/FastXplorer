#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <string>
#include <vector>
#include "raylib.h"

using namespace std;

struct Course {
    int id;
    string name;
    string type; // "Theory" or "Lab"
    int year;
    
    Course(int i, const string& n, const string& t, int y) 
        : id(i), name(n), type(t), year(y) {}
};

struct TimeSlot {
    int startHour; // 8-17 (8:00 AM - 5:00 PM)
    string building; // "R-1" through "R-7" for Theory, "Lab 1", "Lab 3", "Lab 4" for Labs
    Course* course; // pointer to assigned course
    
    TimeSlot() : startHour(-1), building(""), course(nullptr) {}
    TimeSlot(int hour, const string& bldg, Course* c) 
        : startHour(hour), building(bldg), course(c) {}
};

// array based min heap for course scheduling
class CourseHeap {
private:
    TimeSlot* heapArray;
    int capacity;
    int heapSize;
    
    int getParentIndex(int index) { return (index - 1) / 2; }
    int getLeftChildIndex(int index) { return 2 * index + 1; }
    int getRightChildIndex(int index) { return 2 * index + 2; }
    
    void heapifyUp(int index) {
        while (index > 0) {
            int parentindex = getParentIndex(index);
            if (heapArray[index].startHour < heapArray[parentindex].startHour) {
                TimeSlot temp = heapArray[index];
                heapArray[index] = heapArray[parentindex];
                heapArray[parentindex] = temp;
                index = parentindex;
            } else {
                break;
            }
        }
    }
    
    void heapifyDown(int index) {
        while (true) {
            int smallest = index;
            int leftindex = getLeftChildIndex(index);
            int rightindex = getRightChildIndex(index);
            
            if (leftindex < heapSize && heapArray[leftindex].startHour < heapArray[smallest].startHour) {
                smallest = leftindex;
            }
            
            if (rightindex < heapSize && heapArray[rightindex].startHour < heapArray[smallest].startHour) {
                smallest = rightindex;
            }
            
            if (smallest != index) {
                TimeSlot temp = heapArray[index];
                heapArray[index] = heapArray[smallest];
                heapArray[smallest] = temp;
                index = smallest;
            } else {
                break;
            }
        }
    }
    
public:
    CourseHeap(int maxSize) : capacity(maxSize), heapSize(0) {
        heapArray = new TimeSlot[capacity];
    }
    
    ~CourseHeap() {
        delete[] heapArray;
    }
    
    void insert(const TimeSlot& slot) {
        if (heapSize >= capacity) return;
        
        heapArray[heapSize] = slot;
        heapifyUp(heapSize);
        heapSize++;
    }
    
    TimeSlot extractMin() {
        if (heapSize == 0) return TimeSlot();
        
        TimeSlot minSlot = heapArray[0];
        heapSize--;
        
        if (heapSize > 0) {
            heapArray[0] = heapArray[heapSize];
            heapifyDown(0);
        }
        
        return minSlot;
    }
    
    TimeSlot peekMin() const {
        return heapSize > 0 ? heapArray[0] : TimeSlot();
    }
    
    bool isEmpty() const { return heapSize == 0; }
    int size() const { return heapSize; }
    
    TimeSlot getAt(int index) const {
        return (index >= 0 && index < heapSize) ? heapArray[index] : TimeSlot();
    }
    
    void clear() { heapSize = 0; }
    
    // use temp array passed by ref
    void toSortedArray(vector<TimeSlot>& output) const {
        output.clear();
        for (int i = 0; i < heapSize; i++) {
            output.push_back(heapArray[i]);
        }
        // selection sort
        for (int i = 0; i < (int)output.size() - 1; i++) {
            for (int j = i + 1; j < (int)output.size(); j++) {
                if (output[j].startHour < output[i].startHour) {
                    TimeSlot temp = output[i];
                    output[i] = output[j];
                    output[j] = temp;
                }
            }
        }
    }
};

class Scheduler {
private:
    vector<Course> year1Courses;
    vector<Course> year2Courses;
    vector<Course> year3Courses;
    vector<Course> year4Courses;
    
    CourseHeap* scheduleHeap; // min heap for course scheduling
    vector<TimeSlot> completedSlots; // completed classes
    int selectedYear;
    int currentSlot;
    
    // mai hardcode kerdoonga in cpp :/
    vector<string> theoryRooms;
    vector<string> labRooms;
    
    bool loadCoursesFromFile(const string& filepath);
    string getRandomRoom(const string& type);
    
public:
    Scheduler();
    ~Scheduler();
    
    bool initialize(const string& coursesFile);
    bool registerYear(int year);
    void regenerateSchedule();
    
    int getSelectedYear() const { return selectedYear; }
    int getCurrentSlot() const { return currentSlot; }
    
    TimeSlot* getCurrentSlotInfo();
    void advanceSlot();
    
    bool saveToCSV(const string& filepath);
    bool loadFromCSV(const string& filepath);
    
    void draw(Font font = GetFontDefault());
};

#endif
