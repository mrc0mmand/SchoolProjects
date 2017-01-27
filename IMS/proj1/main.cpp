#include <iostream>
#include <iomanip>
#include <locale>
#include <string>
#include <vector>
#include <cmath>
#include <assert.h>
#include "simlib.h"

#ifdef IMS_DEBUG
    #define dbgout std::cout
#else
    std::ostream nullsink(0);
    #define dbgout nullsink
#endif

#define SIMULATION_MINUTE 60.0
#define SIMULATION_HOUR (SIMULATION_MINUTE * 60.0)
#define SIMULATION_DAY (SIMULATION_HOUR * 24.0)
#define SIMULATION_WEEK (SIMULATION_DAY * 7.0)
#define SIMULATION_MONTH (SIMULATION_WEEK * 4.0)
#define SIMULATION_TIME ((SIMULATION_WEEK * 53.0) - SIMULATION_HOUR)
#define COLLECTION_PER_PERSON (SIMULATION_MINUTE / 4.0)
// Year 2010
// 272 kg per person
#define WASTE_PER_PERSON 0.745
// 867 kc per person
#define COST_PER_KG 0.314
#define GARBAGE_TRUCKS 2
// Garbage truck capacity (kg)
#define GARBAGE_TRUCK_CAP 4000.0

typedef struct
{
    float dumped = 0;
    float burned = 0;
    float recycled = 0;
    float composted = 0;
    float total = 0;
} WasteStatistics;

typedef struct
{
    unsigned int start;
    unsigned int end;
    unsigned int next;
    double time;
    double move_time;
    float waste;
    Facility taken;
} TruckData;

// Forward declaration for following std::vector
class Building;

Facility workingHours("Working hours");
Facility wasteProcessing("Waste processing");
Store garbageTrucks("Gargbage trucks", GARBAGE_TRUCKS);
Histogram histCollectionTime("Collection time per building (minutes)", 0,
        1.5, 15);
Histogram histCollectionPerWeek("Total collection time per week (hours)",
        32, 2, 10);
Histogram histWastePerWeek("Waste per week (kilograms)", 27500, 2500, 8);
TruckData truckData[GARBAGE_TRUCKS];
WasteStatistics wasteStats;
std::vector<Building*> buildings;
float wasteCollected = 0;
float weekWaste = 0;
double weekCollection = 0;
int failedCollections = 0;

int current_week()
{
    return int(floor(Time/SIMULATION_WEEK));
}

int current_day()
{
    return int(floor(Time/SIMULATION_DAY));
}

/**
 * @brief Building object
 * @details An object simulating a building (house, factory, ...) and its
 *          waste production
 */
class Building : public Event
{
public:
    typedef enum {
        HOUSE = 0,
        SMALL_FACT = 1,
        MEDIUM_FACT = 2,
        LARGE_FACT = 3,
    } TYPE;

    Building(TYPE t, double ttn, const std::string &n, unsigned int inh = 0) :
        type(t), time_to_next(ttn), name(n), inhabitants(inh),
        waste_produced(0) {}

    float GetWaste() {
        return waste_produced;
    }

    float CollectWaste() {
        float w = waste_produced;
        waste_produced = 0;
        waste_collected = true;
        return w;
    }

    bool WasteCollected() {
        return waste_collected;
    }

    double CollectionTime() {
        double t;

        switch(type) {
        case HOUSE:
            t = SIMULATION_MINUTE * (inhabitants / 10 + 1);
            break;
        case SMALL_FACT:
            t = SIMULATION_MINUTE * 10;
            break;
        case MEDIUM_FACT:
            t = SIMULATION_MINUTE * 15;
            break;
        case LARGE_FACT:
            t = SIMULATION_MINUTE * 20;
            break;
        default:
            std::cerr << "Invalid building type" << std::endl;
            abort();
        }

        return t;
    }

    double TimeToNext() {
        return time_to_next;
    }

private:
    TYPE type;
    double time_to_next;
    std::string name;
    unsigned int inhabitants;
    float waste_produced;
    bool waste_collected = false;

    void Behavior() {
        AddWaste();
        dbgout << "[WEEK " << current_week() << "] Object '" << name
                  << "' produced " << waste_produced
                  << " kilograms of waste" << std::endl;
        Activate(Time+SIMULATION_WEEK);
    }

    void AddWaste() {
        waste_collected = false;

        switch(type) {
        case HOUSE:
            waste_produced += inhabitants * WASTE_PER_PERSON;
            break;
        case SMALL_FACT:
            waste_produced = Uniform(500,999);
            break;

        }
    }

};

/**
 * @brief An object simulating one real-life day
 */
class WorkingHours : public Process
{
private:
    void Behavior() {
        int work_hours = 8;
        int nonwork_hours = 16;
        int curr_day = (current_day() % 7) + 1;
        if(curr_day > 5 && curr_day <= 7) {
            // Skip Saturday & Sunday
            work_hours = 0;
            nonwork_hours = 24;
        }

        if(work_hours > 0) {
            dbgout << "[DAY " << curr_day << "] WORKING HOURS"
                      << std::endl;
            Wait(SIMULATION_HOUR * work_hours);
        }
        dbgout << "[DAY " << curr_day << "] NON-WORKING HOURS"
                  << std::endl;
        Seize(workingHours, 1);
        Wait(SIMULATION_HOUR * nonwork_hours);
        Release(workingHours);
    }
};

class DaySchedule : public Event
{
private:
    void Behavior() {
        if((current_day() + 1) % 7 == 0) {
            bool failed = 0;
            // End of the week - collect data for statistics
            histWastePerWeek(weekWaste);
            weekWaste = 0;

            // Reset all garbage trucks' last positions
            for(unsigned int i = 0; i < GARBAGE_TRUCKS; i++) {
                if(truckData[i].next != truckData[i].end)
                    failed = true;

                truckData[i].next = truckData[i].start;
            }

            histCollectionPerWeek(weekCollection / SIMULATION_HOUR);
            weekCollection = 0;

            if(failed > 0)
                failedCollections++;
        }

        (new WorkingHours)->Activate();
        Activate(Time+SIMULATION_DAY);
    }
};

class GarbageTruck : public Process
{
public:
    float waste = 0;

    GarbageTruck(int id) : truck_id(id) {}

private:
    int truck_id;

    void Behavior() {
        double t;
        double total_time = 0;
        Seize(workingHours);
        Seize(truckData[truck_id].taken);
        Enter(garbageTrucks, 1);
        Release(workingHours);

        t = Exponential(SIMULATION_MINUTE * 15);
        Wait(t);
        total_time += t;
        truckData[truck_id].move_time += t;

        for(unsigned int i = truckData[truck_id].start;
                i < truckData[truck_id].end; i++) {
            if(buildings[i]->WasteCollected())
                continue;

            float w = buildings[i]->GetWaste();
            assert(w <= GARBAGE_TRUCK_CAP);
            if((waste + w) < GARBAGE_TRUCK_CAP) {
                // Garbage collection
                waste += buildings[i]->CollectWaste();
                t = Exponential(buildings[i]->CollectionTime());
                Wait(t);
                total_time += t;
                truckData[truck_id].next++;;
                histCollectionTime(t / SIMULATION_MINUTE);
                dbgout << "[TRUCK #" << truck_id << "] Collected " << w
                       << " kg of waste" << std::endl;
            } else {
                // Truck is full
                break;
            }

            if(workingHours.Busy()) {
                // Return back
                break;
            }

            t = Exponential(buildings[i]->TimeToNext());
            Wait(t);
            total_time += t;
            truckData[truck_id].move_time += t;
        }

        dbgout << "[TRUCK #" << truck_id << "] Returning back" << std::endl;
        t = Exponential(SIMULATION_MINUTE * 25);
        Wait(t);
        total_time += t;
        truckData[truck_id].move_time += t;
        dbgout << "[TRUCK #" << truck_id << "] TOTAL COLLECTED: " << waste
               << " kg" << std::endl;

        truckData[truck_id].time += total_time;
        weekCollection += total_time;
        wasteCollected += waste;
        truckData[truck_id].waste += waste;;
        weekWaste += waste;
        Leave(garbageTrucks, 1);
        Release(truckData[truck_id].taken);
    }
};

class Trucks : public Event
{
private:
    void Behavior() {
        if(!workingHours.Busy() && !garbageTrucks.Full()) {
            for(unsigned int i = 0; i < GARBAGE_TRUCKS; i++) {
                unsigned int rem = truckData[i].end - truckData[i].next;
                if(!truckData[i].taken.Busy() && rem > 0) {
                    (new GarbageTruck(i))->Activate();
                    break;
                }
            }
        }

        Activate(Time + SIMULATION_MINUTE);
    }
};

class ProcessWaste : public Process
{
private:
    void Behavior() {
        Seize(wasteProcessing);
        float w = wasteCollected;
        Wait(Exponential(SIMULATION_HOUR * 3));
        wasteStats.dumped += w * 0.56;
        wasteStats.recycled += w * 0.23;
        wasteStats.burned += w * 0.18;
        wasteStats.composted += w * 0.03;
        wasteStats.total += w;
        wasteCollected -= w;
        Release(wasteProcessing);
    }
};

class WasteProcessing : public Event
{
private:
    void Behavior() {
        if(wasteCollected > 0.0 && !wasteProcessing.Busy()) {
            (new ProcessWaste)->Activate();
        }

        Activate(Time + SIMULATION_MINUTE);
    }
};

void add_house(const std::string &name, int inhabitants, double ttm)
{
    Building *b;
    b = new Building(Building::HOUSE, ttm, name, inhabitants);
    b->Activate();
    buildings.push_back(b);
}

void add_fact(Building::TYPE t, const std::string &name, double ttm)
{
    Building *b;
    b = new Building(t, ttm, name);
    b->Activate();
    buildings.push_back(b);
}

int main(void)
{
    Init(0, SIMULATION_TIME);

    // MAP
    // Street 1
    add_house("School 1", 300, SIMULATION_MINUTE);
    add_house("House 1", 4, SIMULATION_MINUTE);
    add_house("House 2", 6, SIMULATION_MINUTE);
    add_house("House 3", 4, SIMULATION_MINUTE);
    add_house("House 4", 6, SIMULATION_MINUTE);
    add_house("House 5", 8, SIMULATION_MINUTE);
    add_house("House 6", 10, SIMULATION_MINUTE);
    add_house("House 7", 6, SIMULATION_MINUTE);
    add_house("House 8", 4, SIMULATION_MINUTE);
    add_house("House 9", 5, SIMULATION_MINUTE);
    add_house("House 10", 2, SIMULATION_MINUTE * 2);
    // Street 2
    add_house("House 1", 8, SIMULATION_MINUTE);
    add_house("House 2", 4, SIMULATION_MINUTE);
    add_house("House 3", 6, SIMULATION_MINUTE);
    add_house("House 4", 6, SIMULATION_MINUTE);
    add_house("House 5", 7, SIMULATION_MINUTE);
    add_house("House 6", 4, SIMULATION_MINUTE);
    add_house("House 7", 4, SIMULATION_MINUTE);
    add_house("House 8", 8, SIMULATION_MINUTE);
    add_house("House 9", 8, SIMULATION_MINUTE);
    add_house("House 10", 8, SIMULATION_MINUTE);
    add_house("House 11", 6, SIMULATION_MINUTE);
    add_house("House 12", 10, SIMULATION_MINUTE);
    add_house("House 13", 4, SIMULATION_MINUTE);
    add_house("House 14", 6, SIMULATION_MINUTE);
    add_house("House 15", 4, SIMULATION_MINUTE);
    add_house("House 16", 8, SIMULATION_MINUTE * 2);
    // Street 3
    add_house("House 1", 18, SIMULATION_MINUTE);
    add_house("House 2", 10, SIMULATION_MINUTE);
    add_house("House 3", 4, SIMULATION_MINUTE);
    add_house("House 4", 8, SIMULATION_MINUTE);
    add_house("House 5", 6, SIMULATION_MINUTE);
    add_house("House 6", 10, SIMULATION_MINUTE);
    add_house("House 7", 6, SIMULATION_MINUTE);
    add_house("House 8", 6, SIMULATION_MINUTE);
    add_house("House 9", 8, SIMULATION_MINUTE * 3);
    // Street 4
    add_house("House 1", 16, SIMULATION_MINUTE);
    add_house("House 2", 15, SIMULATION_MINUTE);
    add_house("House 3", 12, SIMULATION_MINUTE);
    add_house("House 4", 14, SIMULATION_MINUTE);
    add_house("House 5", 6, SIMULATION_MINUTE);
    add_house("House 6", 12, SIMULATION_MINUTE);
    add_house("House 7", 8, SIMULATION_MINUTE);
    add_house("House 8", 8, SIMULATION_MINUTE);
    add_house("House 9", 14, SIMULATION_MINUTE);
    add_house("House 10", 6, SIMULATION_MINUTE);
    add_house("House 11", 6, SIMULATION_MINUTE);
    add_house("House 12", 12, SIMULATION_MINUTE);
    add_house("House 13", 8, SIMULATION_MINUTE);
    add_house("House 14", 10, SIMULATION_MINUTE);
    add_house("House 15", 8, SIMULATION_MINUTE);
    add_house("House 16", 8, SIMULATION_MINUTE);
    add_house("House 17", 8, SIMULATION_MINUTE);
    add_house("House 18", 6, SIMULATION_MINUTE * 2);
    add_house("House 19", 3, SIMULATION_MINUTE);
    add_house("House 20", 6, SIMULATION_MINUTE);
    add_house("House 21", 4, SIMULATION_MINUTE);
    add_house("House 22", 3, SIMULATION_MINUTE);
    add_house("House 23", 4, SIMULATION_MINUTE);
    // Street 5
    add_fact(Building::SMALL_FACT, "Fact 1", SIMULATION_MINUTE * 2);
    add_fact(Building::SMALL_FACT, "Fact 2", SIMULATION_MINUTE * 3);
    add_house("Block 1", 90, SIMULATION_MINUTE * 5);
    add_house("Block 2", 90, SIMULATION_MINUTE * 5);
    add_fact(Building::SMALL_FACT, "Fact 3", SIMULATION_MINUTE * 2);
    add_fact(Building::SMALL_FACT, "Fact 4", SIMULATION_MINUTE * 2);
    add_house("Block 3", 90, SIMULATION_MINUTE * 5);
    add_house("House 1", 6, SIMULATION_MINUTE);
    add_house("Block 4", 90, SIMULATION_MINUTE * 5);
    add_fact(Building::SMALL_FACT, "Fact 5", SIMULATION_MINUTE * 10);
    add_fact(Building::SMALL_FACT, "Fact 6", SIMULATION_MINUTE * 7);
    add_house("Block 5", 60, SIMULATION_MINUTE);
    add_house("Block 6", 75, SIMULATION_MINUTE);
    add_house("Block 7", 50, SIMULATION_MINUTE);
    add_house("Block 8", 50, SIMULATION_MINUTE);
    add_house("House 2", 8, SIMULATION_MINUTE * 3);
    // Street 6
    add_fact(Building::SMALL_FACT, "Fact 1", SIMULATION_MINUTE * 2);
    add_fact(Building::SMALL_FACT, "Fact 2", SIMULATION_MINUTE * 2);
    add_house("Block 1", 30, SIMULATION_MINUTE);
    add_house("Block 2", 30, SIMULATION_MINUTE);
    add_house("Block 3", 30, SIMULATION_MINUTE);
    add_house("Block 4", 60, SIMULATION_MINUTE);
    add_house("Block 5", 50, SIMULATION_MINUTE);
    add_house("Block 6", 40, SIMULATION_MINUTE);
    add_house("Block 7", 30, SIMULATION_MINUTE);
    add_house("Block 8", 80, SIMULATION_MINUTE);
    // Street 7
    add_fact(Building::SMALL_FACT, "Fact 1", SIMULATION_MINUTE * 2);
    add_house("Block 1", 240, SIMULATION_MINUTE * 5);
    add_fact(Building::SMALL_FACT, "Fact 2", SIMULATION_MINUTE * 2);
    add_house("School 1", 150, SIMULATION_MINUTE * 5);
    add_house("Block 2", 160, SIMULATION_MINUTE * 5);
    add_house("Block 3", 160, SIMULATION_MINUTE * 5);
    add_house("Block 4", 160, SIMULATION_MINUTE * 5);
    add_house("Block 5", 160, SIMULATION_MINUTE * 5);
    add_house("Block 6", 160, SIMULATION_MINUTE * 5);
    add_fact(Building::SMALL_FACT, "Fact 3", SIMULATION_MINUTE * 2);
    // Street 8
    add_house("Block 1", 40, SIMULATION_MINUTE);
    add_house("Block 2", 30, SIMULATION_MINUTE);
    add_house("Block 3", 80, SIMULATION_MINUTE);
    add_house("House 1", 8, SIMULATION_MINUTE);
    add_house("House 2", 6, SIMULATION_MINUTE);
    add_house("House 3", 4, SIMULATION_MINUTE);
    add_house("House 4", 7, SIMULATION_MINUTE);
    add_house("House 5", 4, SIMULATION_MINUTE);
    add_house("House 6", 4, SIMULATION_MINUTE);
    add_house("House 7", 6, SIMULATION_MINUTE);
    add_house("Block 4", 50, SIMULATION_MINUTE);
    add_house("House 8", 8, SIMULATION_MINUTE);
    add_house("House 9", 4, SIMULATION_MINUTE);
    add_house("House 10", 8, SIMULATION_MINUTE);
    add_house("Block 5", 20, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 1", SIMULATION_MINUTE * 2);
    add_house("House 11", 4, SIMULATION_MINUTE);
    add_house("House 12", 10, SIMULATION_MINUTE);
    add_house("Block 6", 50, SIMULATION_MINUTE);
    add_house("House 13", 10, SIMULATION_MINUTE);
    add_house("House 14", 10, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 2", SIMULATION_MINUTE * 2);
    add_house("House 14", 10, SIMULATION_MINUTE);
    // Street 9
    add_house("Block 1", 18, SIMULATION_MINUTE);
    add_house("Block 2", 18, SIMULATION_MINUTE);
    add_house("Block 3", 16, SIMULATION_MINUTE);
    add_house("Block 4", 16, SIMULATION_MINUTE);
    add_house("Block 5", 17, SIMULATION_MINUTE);
    add_house("Block 6", 19, SIMULATION_MINUTE);
    add_house("Block 7", 15, SIMULATION_MINUTE);
    add_house("Block 8", 20, SIMULATION_MINUTE);
    add_house("Block 9", 18, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 1", SIMULATION_MINUTE * 2);
    add_fact(Building::SMALL_FACT, "Fact 2", SIMULATION_MINUTE * 2);
    add_house("Block 10", 30, SIMULATION_MINUTE);
    add_house("Block 11", 32, SIMULATION_MINUTE);
    add_house("Block 12", 12, SIMULATION_MINUTE);
    add_house("Block 13", 18, SIMULATION_MINUTE);
    add_house("Block 14", 22, SIMULATION_MINUTE);
    add_house("Block 15", 20, SIMULATION_MINUTE);
    add_house("Block 16", 20, SIMULATION_MINUTE);
    add_house("Block 17", 12, SIMULATION_MINUTE);
    add_house("Block 18", 14, SIMULATION_MINUTE);
    add_house("Block 19", 20, SIMULATION_MINUTE);
    add_house("Block 20", 20, SIMULATION_MINUTE);
    add_house("Block 21", 15, SIMULATION_MINUTE);
    add_house("Block 22", 13, SIMULATION_MINUTE);
    add_house("Block 23", 22, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 3", SIMULATION_MINUTE * 2);
    add_house("House 1", 4, SIMULATION_MINUTE);
    add_house("House 2", 7, SIMULATION_MINUTE);
    add_house("House 3", 5, SIMULATION_MINUTE);
    add_house("House 4", 6, SIMULATION_MINUTE);
    add_house("Block 24", 12, SIMULATION_MINUTE);
    add_house("Block 25", 10, SIMULATION_MINUTE);
    add_house("Block 26", 20, SIMULATION_MINUTE);
    add_house("Block 27", 15, SIMULATION_MINUTE);
    add_house("Block 28", 25, SIMULATION_MINUTE);
    add_house("Block 29", 40, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 4", SIMULATION_MINUTE * 2);
    add_house("Block 30", 35, SIMULATION_MINUTE);
    add_house("Block 31", 35, SIMULATION_MINUTE);
    add_house("Block 32", 35, SIMULATION_MINUTE);
    add_house("Block 33", 35, SIMULATION_MINUTE);
    add_house("Block 34", 72, SIMULATION_MINUTE);
    add_house("Block 35", 30, SIMULATION_MINUTE);
    add_house("Block 36", 60, SIMULATION_MINUTE);
    add_house("Block 37", 90, SIMULATION_MINUTE);
    add_house("Block 38", 20, SIMULATION_MINUTE);
    add_house("Block 39", 25, SIMULATION_MINUTE * 2);
    add_house("School 1", 200, SIMULATION_MINUTE * 5);
    // Street 10
    add_house("House 1", 5, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 1", SIMULATION_MINUTE * 2);
    add_house("Block 1", 222, SIMULATION_MINUTE * 2);
    add_house("House 2", 8, SIMULATION_MINUTE);
    add_house("Block 2", 50, SIMULATION_MINUTE * 2);
    add_fact(Building::SMALL_FACT, "Fact 2", SIMULATION_MINUTE * 2);
    add_fact(Building::SMALL_FACT, "Fact 3", SIMULATION_MINUTE * 2);
    add_house("Block 3", 30, SIMULATION_MINUTE * 2);
    add_house("House 3", 4, SIMULATION_MINUTE);
    add_house("House 4", 6, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 4", SIMULATION_MINUTE * 2);
    // Street 11
    add_house("House 1", 6, SIMULATION_MINUTE);
    add_house("Block 1", 20, SIMULATION_MINUTE);
    add_house("Block 2", 16, SIMULATION_MINUTE);
    add_house("House 2", 4, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 1", SIMULATION_MINUTE * 2);
    add_house("Block 3", 14, SIMULATION_MINUTE);
    add_house("Block 4", 30, SIMULATION_MINUTE);
    add_house("House 3", 8, SIMULATION_MINUTE);
    add_house("Block 5", 25, SIMULATION_MINUTE);
    add_house("House 4", 5, SIMULATION_MINUTE);
    add_house("House 5", 8, SIMULATION_MINUTE);
    add_house("House 6", 4, SIMULATION_MINUTE);
    add_house("House 7", 5, SIMULATION_MINUTE);
    add_house("Block 6", 12, SIMULATION_MINUTE);
    add_house("Block 7", 15, SIMULATION_MINUTE);
    add_house("Block 8", 15, SIMULATION_MINUTE);
    add_house("Block 9", 60, SIMULATION_MINUTE);
    add_house("Block 10", 40, SIMULATION_MINUTE * 2);
    // Street 12
    add_house("House 1", 6, SIMULATION_MINUTE);
    add_house("House 2", 6, SIMULATION_MINUTE);
    add_house("House 3", 6, SIMULATION_MINUTE);
    add_house("House 4", 4, SIMULATION_MINUTE);
    add_house("House 5", 6, SIMULATION_MINUTE);
    add_house("House 6", 8, SIMULATION_MINUTE);
    add_house("House 7", 8, SIMULATION_MINUTE);
    add_house("House 8", 9, SIMULATION_MINUTE);
    add_house("House 9", 8, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 1", SIMULATION_MINUTE * 2);
    add_fact(Building::SMALL_FACT, "Fact 2", SIMULATION_MINUTE * 2);
    add_fact(Building::SMALL_FACT, "Fact 3", SIMULATION_MINUTE * 2);
    add_house("Block 1", 35, SIMULATION_MINUTE * 2);
    add_house("Block 2", 20, SIMULATION_MINUTE * 2);
    add_house("Block 3", 32, SIMULATION_MINUTE * 2);
    add_fact(Building::SMALL_FACT, "Fact 4", SIMULATION_MINUTE * 2);
    add_fact(Building::SMALL_FACT, "Fact 5", SIMULATION_MINUTE * 2);
    add_fact(Building::SMALL_FACT, "Fact 6", SIMULATION_MINUTE * 2);
    add_house("Block 4", 67, SIMULATION_MINUTE);
    add_house("Block 5", 80, SIMULATION_MINUTE);
    add_house("Block 6", 75, SIMULATION_MINUTE);
    add_house("Block 7", 82, SIMULATION_MINUTE);
    add_house("Block 8", 40, SIMULATION_MINUTE);
    add_house("Block 9", 36, SIMULATION_MINUTE);
    add_house("Block 10", 40, SIMULATION_MINUTE);
    add_house("Block 11", 45, SIMULATION_MINUTE);
    add_house("Block 12", 42, SIMULATION_MINUTE);
    add_house("Block 13", 50, SIMULATION_MINUTE);
    add_house("Block 14", 48, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 7", SIMULATION_MINUTE * 2);
    add_house("Block 15", 51, SIMULATION_MINUTE);
    add_house("Block 16", 53, SIMULATION_MINUTE);
    add_house("Block 17", 49, SIMULATION_MINUTE);
    add_house("School 1", 500, SIMULATION_MINUTE * 2);
    // Street 13
    add_house("Block 1", 22, SIMULATION_MINUTE);
    add_house("Block 2", 28, SIMULATION_MINUTE);
    add_house("Block 3", 42, SIMULATION_MINUTE);
    add_house("Block 4", 20, SIMULATION_MINUTE);
    add_house("Block 5", 32, SIMULATION_MINUTE);
    add_house("Block 6", 34, SIMULATION_MINUTE);
    add_house("Block 7", 33, SIMULATION_MINUTE * 2);
    // Street 14
    add_house("Block 1", 57, SIMULATION_MINUTE);
    add_house("Block 2", 18, SIMULATION_MINUTE);
    add_house("House 1", 4, SIMULATION_MINUTE);
    add_house("Block 3", 12, SIMULATION_MINUTE);
    add_house("Block 4", 15, SIMULATION_MINUTE);
    add_house("House 2", 7, SIMULATION_MINUTE);
    add_house("Block 5", 14, SIMULATION_MINUTE);
    add_house("Block 6", 22, SIMULATION_MINUTE);
    add_house("House 3", 5, SIMULATION_MINUTE);
    add_house("Block 7", 14, SIMULATION_MINUTE);
    add_house("House 4", 8, SIMULATION_MINUTE);
    add_house("Block 8", 25, SIMULATION_MINUTE * 2);
    // Street 15
    add_house("House 1", 7, SIMULATION_MINUTE);
    add_house("Block 1", 14, SIMULATION_MINUTE);
    add_house("House 2", 5, SIMULATION_MINUTE);
    add_house("House 3", 4, SIMULATION_MINUTE);
    add_house("Block 2", 12, SIMULATION_MINUTE);
    add_house("Block 3", 17, SIMULATION_MINUTE);
    add_house("House 4", 4, SIMULATION_MINUTE);
    add_house("Block 4", 30, SIMULATION_MINUTE);
    add_house("Block 5", 20, SIMULATION_MINUTE);
    add_house("House 5", 5, SIMULATION_MINUTE);
    add_house("House 6", 6, SIMULATION_MINUTE);
    add_house("House 7", 4, SIMULATION_MINUTE * 3);
    // Street 16
    add_house("House 1", 3, SIMULATION_MINUTE);
    add_house("House 2", 3, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 1", SIMULATION_MINUTE * 2);
    add_house("House 3", 7, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 2", SIMULATION_MINUTE * 2);
    add_house("House 4", 5, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 3", SIMULATION_MINUTE * 2);
    add_fact(Building::SMALL_FACT, "Fact 4", SIMULATION_MINUTE * 2);
    add_house("House 5", 4, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 5", SIMULATION_MINUTE * 2);
    add_house("House 6", 5, SIMULATION_MINUTE);
    add_house("House 7", 7, SIMULATION_MINUTE);
    add_house("House 8", 6, SIMULATION_MINUTE);
    add_house("House 9", 4, SIMULATION_MINUTE);
    add_house("House 10", 6, SIMULATION_MINUTE);
    add_house("House 11", 5, SIMULATION_MINUTE);
    add_house("House 12", 4, SIMULATION_MINUTE);
    add_house("House 13", 4, SIMULATION_MINUTE);
    add_house("House 14", 5, SIMULATION_MINUTE);
    add_house("House 15", 6, SIMULATION_MINUTE);
    add_house("House 16", 3, SIMULATION_MINUTE);
    add_house("House 17", 2, SIMULATION_MINUTE);
    add_house("House 18", 2, SIMULATION_MINUTE);
    add_house("House 19", 2, SIMULATION_MINUTE);
    add_house("House 20", 5, SIMULATION_MINUTE);
    add_house("House 21", 4, SIMULATION_MINUTE);
    add_house("House 22", 4, SIMULATION_MINUTE);
    add_house("House 23", 3, SIMULATION_MINUTE);
    add_house("Block 1", 14, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 6", SIMULATION_MINUTE * 2);
    add_house("School 1", 250, SIMULATION_MINUTE * 3);
    // Street 17
    add_house("House 1", 6, SIMULATION_MINUTE);
    add_house("House 2", 5, SIMULATION_MINUTE);
    add_house("House 3", 7, SIMULATION_MINUTE);
    add_house("School 1", 200, SIMULATION_MINUTE * 4);
    add_house("House 4", 3, SIMULATION_MINUTE * 3);
    // Street 18
    add_house("Block 1", 40, SIMULATION_MINUTE);
    add_house("Block 2", 32, SIMULATION_MINUTE);
    add_house("House 1", 6, SIMULATION_MINUTE);
    add_house("House 2", 4, SIMULATION_MINUTE);
    add_house("House 3", 7, SIMULATION_MINUTE);
    add_house("House 4", 5, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 1", SIMULATION_MINUTE * 2);
    add_house("Block 3", 24, SIMULATION_MINUTE);
    add_house("Block 4", 14, SIMULATION_MINUTE);
    add_house("Block 5", 12, SIMULATION_MINUTE);
    add_house("Block 6", 18, SIMULATION_MINUTE);
    add_house("Block 7", 15, SIMULATION_MINUTE);
    add_house("Block 8", 27, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 2", SIMULATION_MINUTE * 2);
    // Street 19
    add_house("House 1", 6, SIMULATION_MINUTE);
    add_house("House 2", 5, SIMULATION_MINUTE);
    add_house("House 3", 4, SIMULATION_MINUTE);
    add_house("House 4", 7, SIMULATION_MINUTE);
    add_house("House 5", 6, SIMULATION_MINUTE);
    add_house("Block 1", 40, SIMULATION_MINUTE);
    add_house("Block 2", 35, SIMULATION_MINUTE);
    add_house("Block 3", 20, SIMULATION_MINUTE);
    add_house("Block 4", 97, SIMULATION_MINUTE);
    add_house("Block 5", 89, SIMULATION_MINUTE);
    add_house("House 6", 6, SIMULATION_MINUTE);
    add_house("House 7", 5, SIMULATION_MINUTE);
    add_house("House 8", 4, SIMULATION_MINUTE);
    add_house("House 9", 6, SIMULATION_MINUTE * 2);
    // Street 20
    add_house("House 1", 6, SIMULATION_MINUTE);
    add_house("House 2", 7, SIMULATION_MINUTE);
    add_house("House 3", 3, SIMULATION_MINUTE);
    add_house("House 4", 4, SIMULATION_MINUTE);
    add_house("House 5", 6, SIMULATION_MINUTE);
    add_house("Block 1", 52, SIMULATION_MINUTE);
    add_house("Block 2", 60, SIMULATION_MINUTE);
    add_house("Block 3", 56, SIMULATION_MINUTE);
    add_house("Block 4", 51, SIMULATION_MINUTE);
    add_house("Block 5", 62, SIMULATION_MINUTE);
    add_house("Block 6", 59, SIMULATION_MINUTE);
    add_house("House 6", 7, SIMULATION_MINUTE);
    add_house("House 7", 6, SIMULATION_MINUTE);
    add_house("House 8", 5, SIMULATION_MINUTE);
    add_house("House 9", 7, SIMULATION_MINUTE);
    add_house("House 10", 4, SIMULATION_MINUTE);
    add_house("House 11", 5, SIMULATION_MINUTE);
    add_house("House 12", 6, SIMULATION_MINUTE);
    add_house("Block 7", 66, SIMULATION_MINUTE);
    add_house("Block 8", 61, SIMULATION_MINUTE);
    add_house("Block 9", 69, SIMULATION_MINUTE);
    add_house("Block 10", 60, SIMULATION_MINUTE * 3);
    // Street 21
    add_house("House 1", 6, SIMULATION_MINUTE);
    add_house("House 2", 4, SIMULATION_MINUTE);
    add_house("House 3", 4, SIMULATION_MINUTE);
    add_house("House 4", 4, SIMULATION_MINUTE);
    add_house("House 5", 5, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 1", SIMULATION_MINUTE * 2);
    add_house("House 6", 4, SIMULATION_MINUTE);
    add_house("House 7", 4, SIMULATION_MINUTE);
    add_house("House 8", 4, SIMULATION_MINUTE);
    add_house("House 9", 3, SIMULATION_MINUTE);
    add_house("House 10", 6, SIMULATION_MINUTE);
    add_house("House 11", 7, SIMULATION_MINUTE);
    add_house("House 12", 7, SIMULATION_MINUTE);
    add_house("House 13", 6, SIMULATION_MINUTE);
    add_fact(Building::SMALL_FACT, "Fact 2", SIMULATION_MINUTE * 2);


    // Initialize trucks
    for(unsigned int i = 0; i < GARBAGE_TRUCKS; i++) {
        int x = buildings.size() / GARBAGE_TRUCKS;
        truckData[i].start = i * x;
        truckData[i].end = i * x + x;
        truckData[i].next = truckData[i].start;
        truckData[i].time = 0;
        truckData[i].move_time = 0;
        truckData[i].waste = 0;
    }

    if(truckData[GARBAGE_TRUCKS - 1].end != buildings.size())
        truckData[GARBAGE_TRUCKS - 1].end = buildings.size();

    (new DaySchedule)->Activate();
    (new Trucks)->Activate();
    (new WasteProcessing)->Activate();
    Run();

    // Statistics
    double total_time = 0;
    std::string delim(30, '*');
    std::cout.imbue(std::locale(""));
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Per-truck statistics:" << std::endl << delim << std::endl;
    for(unsigned int i = 0; i < GARBAGE_TRUCKS; i++) {
        total_time += truckData[i].time;
        std::cout << "Truck #" << (i + 1) << std::endl
                  << "\tWaste collected:\t" << truckData[i].waste << " kg"
                  << std::endl
                  << "\tTime on road:\t\t"
                  << (truckData[i].time / SIMULATION_HOUR) << " hours"
                  << std::endl
                  << "\tCost:\t\t\t" << (truckData[i].waste / COST_PER_KG)
                  << " kc" << std::endl;
    }
    std::cout << std::endl
              << "TOTAL TIME:\t\t" << (total_time / SIMULATION_HOUR)
              << " hours" << std::endl
              << "FAILED COLLECTIONS:\t" << failedCollections << std::endl
              << std::endl;
    std::cout << "General statistics:" << std::endl << delim << std::endl;
    std::cout << "Total buildings: " << buildings.size() << std::endl;
    std::cout << "Waste statistics:" << std::endl
              << "Dumped:\t\t" << wasteStats.dumped << " kg" << std::endl
              << "Recycled:\t" << wasteStats.recycled << " kg" << std::endl
              << "Burned:\t\t" << wasteStats.burned << " kg" << std::endl
              << "Composted:\t" << wasteStats.composted << " kg" << std::endl
              << "TOTAL:\t\t" << wasteStats.total << " kg" << std::endl
              << std::endl;

    histCollectionTime.Output();
    histCollectionPerWeek.Output();
    histWastePerWeek.Output();

    return 0;
}
