#include "../headers/schedule.h"
#include "../headers/settings.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <cstdlib>

Scheduler::Scheduler() : scheduleHeap(nullptr), selectedYear(0), currentSlot(0) {
    // hardcoded rooms (must match building names in campus.cpp)
    theoryRooms = {"R-1", "R-2", "R-3", "R-4", "R-5", "R-6", "R-7"};
    labRooms = {"Lab 1", "Lab 3", "Lab 4"};

    srand(static_cast<unsigned>(time(nullptr)));
}

Scheduler::~Scheduler() {
    if (scheduleHeap) {
        delete scheduleHeap;
    }
}

bool Scheduler::initialize(const string& coursesFile) {
    return loadCoursesFromFile(coursesFile);
}

bool Scheduler::loadCoursesFromFile(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        TraceLog(LOG_ERROR, "Failed to open courses file: %s", filepath.c_str());
        return false;
    }
    
    string line;
    int currentYear = 0;
    
    while (getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (line.empty()) continue;
        
        // Check for year header
        if (line.find("Year") == 0) {
            size_t spacePos = line.find(' ');
            if (spacePos != string::npos) {
                currentYear = stoi(line.substr(spacePos + 1));
            }
            continue;
        }
        
        // Parse course line: id, name, type
        stringstream ss(line);
        string idStr, name, type;
        
        getline(ss, idStr, ',');
        getline(ss, name, ',');
        getline(ss, type, ',');
        
        // Trim whitespace from each field
        idStr.erase(0, idStr.find_first_not_of(" \t"));
        idStr.erase(idStr.find_last_not_of(" \t") + 1);
        name.erase(0, name.find_first_not_of(" \t"));
        name.erase(name.find_last_not_of(" \t") + 1);
        type.erase(0, type.find_first_not_of(" \t"));
        type.erase(type.find_last_not_of(" \t") + 1);
        
        if (!idStr.empty() && !name.empty() && !type.empty() && currentYear > 0) {
            int id = stoi(idStr);
            Course course(id, name, type, currentYear);
            
            switch (currentYear) {
                case 1: year1Courses.push_back(course); break;
                case 2: year2Courses.push_back(course); break;
                case 3: year3Courses.push_back(course); break;
                case 4: year4Courses.push_back(course); break;
            }
        }
    }
    
    file.close();
    
    TraceLog(LOG_INFO, "Loaded courses - Y1:%d, Y2:%d, Y3:%d, Y4:%d", 
             year1Courses.size(), year2Courses.size(), 
             year3Courses.size(), year4Courses.size());
    
    return true;
}

string Scheduler::getRandomRoom(const string& type) {
    if (type == "Lab") {
        return labRooms[rand() % labRooms.size()];
    } else {
        return theoryRooms[rand() % theoryRooms.size()];
    }
}

bool Scheduler::registerYear(int year) {
    if (year < 1 || year > 4) {
        return false;
    }
    
    selectedYear = year;
    currentSlot = 1;
    
    // get courses for selected year
    vector<Course>* courses = nullptr;
    switch (year) {
        case 1: courses = &year1Courses; break;
        case 2: courses = &year2Courses; break;
        case 3: courses = &year3Courses; break;
        case 4: courses = &year4Courses; break;
    }
    
    if (!courses || courses->empty()) {
        return false;
    }
    
    // clear old heap and create new
    if (scheduleHeap) delete scheduleHeap;
    scheduleHeap = new CourseHeap(courses->size());

    vector<int> hours = {8,9,10,11,12,13,14,15,16,17}; // available hours

    // randomly assign hours to courses & it insert into min heap
    for (size_t i = 0; i < courses->size() && i < hours.size(); ++i) {
        int randomIndex = rand() % hours.size();
        int selectedHour = hours[randomIndex];
        hours.erase(hours.begin() + randomIndex);
        string room = getRandomRoom((*courses)[i].type); // random room based on type
        scheduleHeap->insert(TimeSlot(selectedHour, room, &(*courses)[i]));
    }

    return true;
}

void Scheduler::regenerateSchedule() {
    if (selectedYear == 0) return;
    // simply re register with same year
    completedSlots.clear();
    currentSlot = 1;
    registerYear(selectedYear);
}

TimeSlot* Scheduler::getCurrentSlotInfo() {
    if (!scheduleHeap || scheduleHeap->isEmpty()) {
        return nullptr;
    }
    static TimeSlot currentTimeSlot;
    currentTimeSlot = scheduleHeap->peekMin();
    return &currentTimeSlot;
}

void Scheduler::advanceSlot() { // next slot
    if (!scheduleHeap || scheduleHeap->isEmpty()) return;
    
    // store completed slot
    TimeSlot completed = scheduleHeap->extractMin();
    completedSlots.push_back(completed);
    currentSlot++;
    
    // if heap empty, recycle completed slots
    if (scheduleHeap->isEmpty()) {
        for (const auto& slot : completedSlots) {
            scheduleHeap->insert(slot);
        }
        completedSlots.clear();
        currentSlot = 1;
    }
}

bool Scheduler::saveToCSV(const string& filepath) {
    ofstream file(filepath);
    if (!file.is_open()) {
        TraceLog(LOG_ERROR, "Failed to open file for writing: %s", filepath.c_str());
        return false;
    }
    
    file << selectedYear << "," << currentSlot << "\n";
    
    // Save completed slots first
    for (const auto& slot : completedSlots) {
        if (slot.course) {
            file << slot.course->id << "," << slot.course->name << "," << slot.course->type << "," << slot.startHour << "," << slot.building << ",1\n";
        }
    }
    
    // Save remaining slots in heap
    if (scheduleHeap) {
        vector<TimeSlot> remaining;
        scheduleHeap->toSortedArray(remaining);
        
        for (const auto& slot : remaining) {
            if (slot.course) {
                file << slot.course->id << "," << slot.course->name << "," << slot.course->type << "," << slot.startHour << "," << slot.building << ",0\n";
            }
        }
    }
    
    file.close();
    TraceLog(LOG_INFO, "Saved schedule to %s", filepath.c_str());
    return true;
}

bool Scheduler::loadFromCSV(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        TraceLog(LOG_WARNING, "No saved schedule found: %s", filepath.c_str());
        return false;
    }
    
    string line;
    if (!getline(file, line)) {
        file.close();
        return false;
    }
    
    stringstream ss(line);
    string yearStr, slotStr;
    getline(ss, yearStr, ',');
    getline(ss, slotStr, ',');
    
    int year = stoi(yearStr);
    currentSlot = stoi(slotStr);
    
    vector<Course>* courses = nullptr;
    switch (year) {
        case 1: courses = &year1Courses; break;
        case 2: courses = &year2Courses; break;
        case 3: courses = &year3Courses; break;
        case 4: courses = &year4Courses; break;
    }
    
    if (!courses || courses->empty()) {
        file.close();
        return false;
    }
    
    selectedYear = year;
    
    if (scheduleHeap) delete scheduleHeap;
    scheduleHeap = new CourseHeap(courses->size());
    completedSlots.clear();
    
    while (getline(file, line)) {
        stringstream slotStream(line);
        string idStr, name, type, hourStr, building, completedStr;
        
        getline(slotStream, idStr, ',');
        getline(slotStream, name, ',');
        getline(slotStream, type, ',');
        getline(slotStream, hourStr, ',');
        getline(slotStream, building, ',');
        getline(slotStream, completedStr, ',');
        
        int courseId = stoi(idStr);
        int hour = stoi(hourStr);
        bool isCompleted = !completedStr.empty() && stoi(completedStr) == 1;
        
        Course* coursePtr = nullptr;
        for (auto& c : *courses) {
            if (c.id == courseId) {
                coursePtr = &c;
                break;
            }
        }
        
        if (coursePtr) {
            TimeSlot slot(hour, building, coursePtr);
            if (isCompleted) {
                completedSlots.push_back(slot);
            } else {
                scheduleHeap->insert(slot);
            }
        }
    }
    
    file.close();
    
    TraceLog(LOG_INFO, "Loaded schedule from %s - Year %d, Slot %d", filepath.c_str(), year, currentSlot);
    return true;
}

void Scheduler::draw(Font font) {
    // Combine completed and remaining slots for full view
    vector<TimeSlot> slots = completedSlots;
    
    // Add remaining slots from heap
    if (scheduleHeap && !scheduleHeap->isEmpty()) {
        vector<TimeSlot> remaining;
        scheduleHeap->toSortedArray(remaining);
        slots.insert(slots.end(), remaining.begin(), remaining.end());
    }
    
    // Sort all slots by time for display with selection sort
    for (int i = 0; i < (int)slots.size() - 1; i++) {
        for (int j = i + 1; j < (int)slots.size(); j++) {
            if (slots[j].startHour < slots[i].startHour) {
                TimeSlot temp = slots[i];
                slots[i] = slots[j];
                slots[j] = temp;
            }
        }
    }
    
    if (slots.empty()) {
        DrawTextEx(font, "No schedule available", Vector2{(float)(WINDOW_WIDTH/2 - 100), (float)(WINDOW_HEIGHT/2)}, 24, 0, RED);
        return;
    }
    
    // Background panel - larger display
    int panelW = WINDOW_WIDTH - 100;
    int panelH = WINDOW_HEIGHT - 100;
    int x = (WINDOW_WIDTH - panelW) / 2;
    int y = (WINDOW_HEIGHT - panelH) / 2;
    DrawRectangleRounded({(float)x, (float)y, (float)panelW, (float)panelH}, 0.08f, 16, Fade(DARKBLUE, 0.8f));
    DrawRectangleRoundedLines({(float)x, (float)y, (float)panelW, (float)panelH}, 0.08f, 16, Fade(RAYWHITE, 0.4f));

    const int fontSize = 32;
    string title = "Timetable - Year " + to_string(selectedYear == 0 ? 1 : selectedYear);
    int titleW = MeasureTextEx(font, title.c_str(), 50, 0).x;
    DrawTextEx(font, title.c_str(), Vector2{(float)(x + (panelW - titleW) / 2), (float)(y + 25)}, 50, 0, RAYWHITE);
    DrawTextEx(font, "Press T to close | G to regenerate Timetable", Vector2{(float)(x + 40), (float)(y + 90)}, 24, 0, LIGHTGRAY);

    // Table headers
    int col1 = x + 50;
    int col2 = col1 + 140;
    int col3 = col2 + 450;
    int col4 = col3 + 280;
    int rowY = y + 160;
    DrawTextEx(font, "Slot", Vector2{(float)col1, (float)rowY}, fontSize, 0, SKYBLUE);
    DrawTextEx(font, "Course", Vector2{(float)col2, (float)rowY}, fontSize, 0, SKYBLUE);
    DrawTextEx(font, "Room", Vector2{(float)col3, (float)rowY}, fontSize, 0, SKYBLUE);
    DrawTextEx(font, "Time", Vector2{(float)col4, (float)rowY}, fontSize, 0, SKYBLUE);

    rowY += 10;
    DrawLine(col1, rowY + 30, x + panelW - 50, rowY + 30, Fade(RAYWHITE, 0.4f));
    rowY += 50;

    for (size_t i = 0; i < slots.size(); i++) {
        const TimeSlot& slot = slots[i];
        Color textColor = (int)i + 1 == currentSlot ? YELLOW : RAYWHITE;
        string slotStr = to_string(i + 1);
        string courseStr = slot.course ? slot.course->name : "";
        string roomStr = slot.building;
        string timeStr = to_string(slot.startHour) + ":00";
        DrawTextEx(font, slotStr.c_str(), Vector2{(float)col1, (float)rowY}, fontSize, 0, textColor);
        DrawTextEx(font, courseStr.c_str(), Vector2{(float)col2, (float)rowY}, fontSize, 0, textColor);
        DrawTextEx(font, roomStr.c_str(), Vector2{(float)col3, (float)rowY}, fontSize, 0, textColor);
        DrawTextEx(font, timeStr.c_str(), Vector2{(float)col4, (float)rowY}, fontSize, 0, textColor);
        rowY += fontSize + 22;
    }
}
