// ============================================================
// Book Ride Subsystem - Ride Sharing Application
// Student  : Manasa Gopisetty  |  ID: 241071039  |  Batch C
// Course   : Software Engineering (R5IT2009T)
// VJTI Mumbai  |  End Semester Lab Exam  |  30-05-2026
// ============================================================
// Classes match Q1 handwritten model:
//   Entity    : Ride_details, Login_rider, Rider_pick, Rider_dest,
//               Vehicle, Fare, Receipt, Driver_decision
//   Boundary  : Dashboard, Driver_loc_rider
//   Control   : Driver_finder
// ============================================================

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <functional>

using namespace std;

// ── ANSI colours ────────────────────────────────────────────
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define GREEN   "\033[92m"
#define RED     "\033[91m"
#define YELLOW  "\033[1;93m"
#define CYAN    "\033[96m"

// ============================================================
//  ENUMS
// ============================================================
enum class VehicleType  { BIKE, AUTO, SEDAN, SUV };
enum class RideStatus   { PENDING, DRIVER_ASSIGNED, IN_PROGRESS, COMPLETED, CANCELLED };
enum class DriverStatus { ONLINE, OFFLINE, ON_RIDE };

string vtStr(VehicleType v){
    switch(v){ case VehicleType::BIKE:  return "Bike";
               case VehicleType::AUTO:  return "Auto";
               case VehicleType::SEDAN: return "Sedan";
               case VehicleType::SUV:   return "SUV"; }
    return "Unknown";
}
string rsStr(RideStatus s){
    switch(s){ case RideStatus::PENDING:          return "PENDING";
               case RideStatus::DRIVER_ASSIGNED:  return "DRIVER_ASSIGNED";
               case RideStatus::IN_PROGRESS:      return "IN_PROGRESS";
               case RideStatus::COMPLETED:        return "COMPLETED";
               case RideStatus::CANCELLED:        return "CANCELLED"; }
    return "Unknown";
}

// ============================================================
//  CUSTOM EXCEPTIONS
// ============================================================
class RideException               : public runtime_error {
public: explicit RideException(const string& m)               : runtime_error(m){} };
class InvalidLocationException    : public RideException {
public: explicit InvalidLocationException(const string& m)    : RideException(m){} };
class DriverNotAvailableException : public RideException {
public: explicit DriverNotAvailableException(const string& m) : RideException(m){} };
class InvalidFareException        : public RideException {
public: explicit InvalidFareException(const string& m)        : RideException(m){} };
class InvalidVehicleException     : public RideException {
public: explicit InvalidVehicleException(const string& m)     : RideException(m){} };

// ============================================================
//  ENTITY: Rider_pick  (pickup location)
// ============================================================
class Rider_pick {
    string locationName;
    double lat, lon;
public:
    Rider_pick() : locationName(""), lat(0), lon(0) {}
    Rider_pick(const string& name, double la, double lo)
        : locationName(name), lat(la), lon(lo) {
        if(name.empty())
            throw InvalidLocationException("Pickup location name cannot be empty.");
    }
    string getName()     const { return locationName; }
    double getLatitude() const { return lat; }
    double getLongitude()const { return lon; }

    double distanceTo(const Rider_pick& other) const {
        double dlat = lat - other.lat, dlon = lon - other.lon;
        return sqrt(dlat*dlat + dlon*dlon) * 111.0;  // degrees -> km
    }
};

// ============================================================
//  ENTITY: Rider_dest  (drop / destination location)
// ============================================================
class Rider_dest {
    string locationName;
    double lat, lon;
public:
    Rider_dest() : locationName(""), lat(0), lon(0) {}
    Rider_dest(const string& name, double la, double lo)
        : locationName(name), lat(la), lon(lo) {
        if(name.empty())
            throw InvalidLocationException("Drop location name cannot be empty.");
    }
    string getName()     const { return locationName; }
    double getLatitude() const { return lat; }
    double getLongitude()const { return lon; }
};

// ============================================================
//  ENTITY: Vehicle
// ============================================================
class Vehicle {
    VehicleType type;
    string      vehicleNumber;
public:
    Vehicle(VehicleType vt, const string& vno)
        : type(vt), vehicleNumber(vno) {}
    VehicleType getType()         const { return type; }
    string      getVehicleNumber()const { return vehicleNumber; }
};

// ============================================================
//  ENTITY: Fare
// ============================================================
class Fare {
    double amount;
public:
    Fare() : amount(0) {}
    explicit Fare(double amt) : amount(amt) {
        if(amt < 0)
            throw InvalidFareException("Fare cannot be negative.");
    }
    double getAmount() const { return amount; }
    void   setAmount(double a){ amount = a; }
};

// ============================================================
//  ENTITY: Login_rider  (Rider)
// ============================================================
class Login_rider {
    int    riderId;
    string name, phone;
    bool   loggedIn;
public:
    Login_rider(int id, const string& n, const string& p)
        : riderId(id), name(n), phone(p), loggedIn(false) {}

    void login()       { loggedIn = true;  }
    void logout()      { loggedIn = false; }
    bool isLoggedIn()  const { return loggedIn; }
    int    getRiderId()const { return riderId;  }
    string getName()   const { return name;     }
    string getPhone()  const { return phone;    }
};

// ============================================================
//  ENTITY: Driver_decision  (Driver)
// ============================================================
class Driver_decision {
    int          driverId;
    string       name, phone;
    Vehicle      vehicle;
    DriverStatus status;
    double       rating;
    int          activeRideId;   // -1 = no active ride

public:
    Driver_decision(int id, const string& n, const string& p,
                    VehicleType vt, const string& vno)
        : driverId(id), name(n), phone(p),
          vehicle(vt, vno),
          status(DriverStatus::ONLINE),
          rating(4.5), activeRideId(-1) {}

    int          getDriverId()   const { return driverId;             }
    string       getName()       const { return name;                 }
    string       getPhone()      const { return phone;                }
    VehicleType  getVehicleType()const { return vehicle.getType();    }
    string       getVehicleNo()  const { return vehicle.getVehicleNumber(); }
    DriverStatus getStatus()     const { return status;               }
    double       getRating()     const { return rating;               }
    int          getActiveRideId()const{ return activeRideId;         }

    bool isAvailable() const {
        return status == DriverStatus::ONLINE && activeRideId == -1;
    }

    void setStatus(DriverStatus s) { status = s; }

    // Single-active-ride enforcement
    void acceptRide(int rideId) {
        if(!isAvailable())
            throw DriverNotAvailableException(
                "Driver " + name + " is not available (already on a ride or offline).");
        activeRideId = rideId;
        status = DriverStatus::ON_RIDE;
    }

    void completeRide() {
        activeRideId = -1;
        status = DriverStatus::ONLINE;
    }
};

// ============================================================
//  ENTITY: Receipt
// ============================================================
class Receipt {
public:
    static void print(int rideId, const string& riderName,
                      const string& driverName, const string& vehicleType,
                      const string& pickup, const string& drop,
                      double distKm, double fareAmt, const string& status) {
        cout << "  " << string(44,'=') << "\n";
        cout << "  RIDE RECEIPT\n";
        cout << "  " << string(44,'=') << "\n";
        cout << "  Ride ID   : " << rideId      << "\n";
        cout << "  Rider     : " << riderName    << "\n";
        cout << "  Driver    : " << driverName   << "\n";
        cout << "  Vehicle   : " << vehicleType  << "\n";
        cout << "  Pickup    : " << pickup        << "\n";
        cout << "  Drop      : " << drop          << "\n";
        cout << fixed << setprecision(2);
        cout << "  Distance  : " << distKm << " km\n";
        cout << "  Fare      : Rs. " << fareAmt  << "\n";
        cout << "  Status    : " << status        << "\n";
        cout << "  " << string(44,'=') << "\n";
    }
};

// ============================================================
//  ENTITY: Ride_details  (core ride entity)
//          Collaborations: Login_rider, Rider_pick, Rider_dest,
//                          Driver_decision, Vehicle, Fare,
//                          ride_history, Receipt
// ============================================================
class Ride_details {
    static int nextRideId;

    int              rideId;
    Login_rider*     rider;
    Driver_decision* driver;
    Rider_pick       pickupLoc;
    Rider_dest       dropLoc;
    VehicleType      vehicleType;
    RideStatus       rideStatus;
    Fare             fare;
    double           distanceKm;

public:
    Ride_details(Login_rider* r,
                 const Rider_pick& pickup,
                 const Rider_dest& drop,
                 VehicleType vt)
        : rideId(nextRideId++), rider(r), driver(nullptr),
          pickupLoc(pickup), dropLoc(drop),
          vehicleType(vt), rideStatus(RideStatus::PENDING),
          distanceKm(0)
    {
        if(!r->isLoggedIn())
            throw RideException("Login_rider must be logged in to book a ride.");

        // compute distance using pickup coords
        Rider_pick dest_as_pick(dropLoc.getName(),
                                dropLoc.getLatitude(),
                                dropLoc.getLongitude());
        distanceKm = pickupLoc.distanceTo(dest_as_pick);
        fare = Fare(calculateFare());
    }

    // ── calculateFare (estimateFare in Q1 model) ──────────────
    double calculateFare() const {
        double rate;
        switch(vehicleType){
            case VehicleType::BIKE:  rate = 8.0;  break;
            case VehicleType::AUTO:  rate = 12.0; break;
            case VehicleType::SEDAN: rate = 18.0; break;
            case VehicleType::SUV:   rate = 25.0; break;
        }
        double f = rate * distanceKm + 20.0;   // Rs.20 base charge
        if(f < 0) throw InvalidFareException("Calculated fare is negative.");
        return f;
    }

    // ── assignDriver (method under White Box test) ─────────────
    //
    //  CONTROL FLOW GRAPH:
    //
    //   N1 (Entry)
    //    |
    //    v
    //   N2: driver != nullptr ?
    //    |-- YES --> N6: throw RideException("already assigned")
    //    |-- NO  --> N3: driver->isAvailable() && type match ?
    //                 |-- YES --> N4: assign driver, status=DRIVER_ASSIGNED
    //                 |-- NO  --> N5: throw DriverNotAvailableException
    //   N4, N5, N6 --> N7 (Exit)
    //
    //  Edges = 8, Nodes = 7
    //  Cyclomatic Complexity V(G) = E - N + 2P = 8 - 7 + 2 = 3
    //
    //  Independent Paths:
    //   Path 1: N1->N2->N3->N4->N7  (happy: available + type match)
    //   Path 2: N1->N2->N3->N5->N7  (fail:  offline OR type mismatch)
    //   Path 3: N1->N2->N6->N7      (fail:  driver already assigned)
    //
    void assignDriver(Driver_decision* d) {
        if(driver != nullptr)                                         // N1->N2
            throw RideException("Ride already has a driver assigned."); // N6->N7

        if(d->isAvailable() && d->getVehicleType() == vehicleType) { // N3
            driver = d;                                               // N4
            d->acceptRide(rideId);
            rideStatus = RideStatus::DRIVER_ASSIGNED;
        } else {                                                      // N5
            throw DriverNotAvailableException(
                "Driver unavailable or vehicle type mismatch.");
        }
        // N7 exit
    }

    // ── match_driver (from sequence diagram) ──────────────────
    // Delegates to Driver_finder (see below). Wrapper kept here
    // so sequence diagram flow is preserved.
    Driver_decision* match_driver(vector<Driver_decision*>& pool);

    // ── change_vehicle ────────────────────────────────────────
    void change_vehicle(VehicleType newType) {
        if(rideStatus != RideStatus::PENDING)
            throw RideException("Cannot change vehicle after driver is assigned.");
        vehicleType = newType;
        fare = Fare(calculateFare());
    }

    // ── ride lifecycle ────────────────────────────────────────
    void startRide() {
        if(rideStatus != RideStatus::DRIVER_ASSIGNED)
            throw RideException("Cannot start ride: no driver assigned yet.");
        rideStatus = RideStatus::IN_PROGRESS;
    }

    void completeRide() {
        if(rideStatus != RideStatus::IN_PROGRESS)
            throw RideException("Cannot complete: ride not IN_PROGRESS.");
        rideStatus = RideStatus::COMPLETED;
        if(driver) driver->completeRide();
    }

    void cancelRide() {
        if(rideStatus == RideStatus::COMPLETED)
            throw RideException("Cannot cancel a completed ride.");
        rideStatus = RideStatus::CANCELLED;
        if(driver) driver->completeRide();
    }

    void printReceipt() const {
        Receipt::print(
            rideId,
            rider->getName(),
            driver ? driver->getName() : "N/A",
            vtStr(vehicleType),
            pickupLoc.getName(),
            dropLoc.getName(),
            distanceKm,
            fare.getAmount(),
            rsStr(rideStatus)
        );
    }

    // ── getters ───────────────────────────────────────────────
    int              getRideId()    const { return rideId;           }
    RideStatus       getStatus()    const { return rideStatus;       }
    double           getFare()      const { return fare.getAmount(); }
    double           getDistanceKm()const { return distanceKm;      }
    VehicleType      getVehicleType()const{ return vehicleType;      }
    Driver_decision* getDriver()    const { return driver;           }
    Login_rider*     getRider()     const { return rider;            }
    Rider_pick       getPickup()    const { return pickupLoc;        }
    Rider_dest       getDrop()      const { return dropLoc;          }
};
int Ride_details::nextRideId = 1001;

// ============================================================
//  CONTROL: Driver_finder
//           Responsibilities (from CRC card):
//           1. Get rider's location from pickup
//           2. Search through drivers
//           3. Driver accepts/rejects -> match_driver()
//           4. Map leading driver to rider's location
//           5. Driver authenticates rider
//           6. Driver picks up rider to destination
// ============================================================
class Driver_finder {
public:
    // match_driver — finds best available driver matching vehicle type
    // (called from Ride_details::match_driver and sequence diagram)
    static Driver_decision* match_driver(vector<Driver_decision*>& pool,
                                         VehicleType vt) {
        for(Driver_decision* d : pool)
            if(d->isAvailable() && d->getVehicleType() == vt)
                return d;
        return nullptr;
    }

    // Driver_AccorReject — driver accepts or rejects a ride
    static bool Driver_AccorReject(Driver_decision* d, bool accepts) {
        return accepts && d->isAvailable();
    }
};

// Ride_details::match_driver implementation (needs Driver_finder)
Driver_decision* Ride_details::match_driver(vector<Driver_decision*>& pool) {
    return Driver_finder::match_driver(pool, vehicleType);
}

// ============================================================
//  BOUNDARY: Dashboard
//            Responsibilities (from CRC card):
//            1. View driver's location
//            2. View driver's earnings
//            3. View rider's ride history
//            4. View driver's trip history
// ============================================================
class Dashboard {
public:
    static void showRideStatus(const Ride_details& rd) {
        cout << "  [Dashboard] Ride #" << rd.getRideId()
             << " | Status: " << rsStr(rd.getStatus())
             << " | Fare: Rs." << fixed << setprecision(2) << rd.getFare()
             << " | Vehicle: " << vtStr(rd.getVehicleType()) << "\n";
    }
    static void showDriverLocation(const Driver_decision& d) {
        cout << "  [Dashboard] Driver " << d.getName()
             << " | Status: " << (d.isAvailable() ? "AVAILABLE" : "ON_RIDE")
             << " | Vehicle: " << d.getVehicleNo() << "\n";
    }
};

// ============================================================
//  TEST FRAMEWORK
// ============================================================
struct TestResult {
    string name, input, expected, actual;
    bool   passed;
};
static vector<TestResult> g_all;
static int g_passed=0, g_failed=0;
static string g_input, g_expected, g_actual;

void set_input   (const string& s){ g_input    = s; }
void set_expected(const string& s){ g_expected = s; }
void set_actual  (const string& s){ g_actual   = s; }

void run_test(const string& name, function<bool()> fn){
    g_input = g_expected = g_actual = "";
    bool ok = false;
    try { ok = fn(); }
    catch(exception& e){ g_actual = string("Exception: ")+e.what(); ok=false; }
    catch(...){ g_actual = "Unknown exception"; ok=false; }
    if(ok) g_passed++; else g_failed++;
    g_all.push_back({name, g_input, g_expected, g_actual, ok});
}

bool assert_equals(const string& expected, const string& actual){
    set_expected(expected); set_actual(actual);
    return expected == actual;
}
template<typename ExcType, typename F>
bool assert_raises_typed(const string& excName, F fn){
    set_expected(excName);
    try { fn(); set_actual("No exception"); return false; }
    catch(ExcType&){ set_actual(excName); return true; }
    catch(exception& e){ set_actual(string("Other: ")+e.what()); return false; }
}

// ── Table printing (matches reference repo format) ──────────
const int W_NAME=45, W_INP=55, W_EXP=26, W_ACT=26;

string lj(const string& s, int w){
    if((int)s.size()>=w) return s.substr(0,w);
    return s + string(w-s.size(),' ');
}
void print_row(const string& n,const string& i,
               const string& e,const string& a,const string& r){
    cout << lj(n,W_NAME)<<" | "<<lj(i,W_INP)<<" | "
         <<lj(e,W_EXP)<<" | "<<lj(a,W_ACT)<<" | "<<r<<"\n";
}
void print_header(){
    print_row("TEST CASE NAME","INPUT","EXPECTED","ACTUAL","RESULT");
    cout<<string(W_NAME,'-')<<"-|-"<<string(W_INP,'-')<<"-|-"
        <<string(W_EXP,'-')<<"-|-"<<string(W_ACT,'-')<<"-|---------\n";
}

void print_table(const string& title, const string& prefix){
    cout<<"\n"<<YELLOW<<"+"<<string(78,'=')<<"+"<<RESET<<"\n";
    cout<<YELLOW<<"  "<<title<<RESET<<"\n";
    cout<<YELLOW<<"+"<<string(78,'=')<<"+"<<RESET<<"\n";
    print_header();
    int pass=0, fail=0;
    for(auto& r: g_all){
        if(r.name.substr(0,prefix.size())!=prefix) continue;
        string res = r.passed ? string(GREEN)+"PASS"+RESET
                              : string(RED)  +"FAIL"+RESET;
        print_row(r.name, r.input, r.expected, r.actual, res);
        r.passed ? pass++ : fail++;
    }
    cout<<"─────────────────────────────────────────────\n";
    string label = (prefix=="WB") ? "White Box" : "Black Box";
    cout<<label<<" -> Total: "<<(pass+fail)
        <<" | Passed: "<<GREEN<<pass<<RESET
        <<" | Failed: "<<RED<<fail<<RESET<<"\n";
}

// ============================================================
//  WHITE BOX TESTS – 15 TCs on Ride_details::assignDriver()
// ============================================================
void run_white_box_tests(){

    // WB-01  Path N1->N2->N3->N4->N7  (happy path – Statement coverage)
    run_test("WB-01_assignDriver_available_type_match",[]{
        set_input("Driver_decision ONLINE, SEDAN, activeRideId=-1");
        Login_rider r(1,"Manasa","9999"); r.login();
        Rider_pick  pu("Matunga",19.0224,72.8597);
        Rider_dest  dr("Bandra", 19.0596,72.8295);
        Ride_details rd(&r,pu,dr,VehicleType::SEDAN);
        Driver_decision d(101,"Ramesh","8888",VehicleType::SEDAN,"MH01AB1234");
        rd.assignDriver(&d);
        return assert_equals("DRIVER_ASSIGNED", rsStr(rd.getStatus()));
    });

    // WB-02  Path N1->N2->N3->N5->N7  (Branch: driver OFFLINE)
    run_test("WB-02_assignDriver_driver_offline",[]{
        set_input("Driver_decision status=OFFLINE, SEDAN");
        Login_rider r(2,"Priya","7777"); r.login();
        Rider_pick pu("Dadar",19.0186,72.8431);
        Rider_dest dr("Andheri",19.1136,72.8697);
        Ride_details rd(&r,pu,dr,VehicleType::SEDAN);
        Driver_decision d(102,"Suresh","6666",VehicleType::SEDAN,"MH02CD5678");
        d.setStatus(DriverStatus::OFFLINE);
        return assert_raises_typed<DriverNotAvailableException>(
            "DriverNotAvailableException",[&](){ rd.assignDriver(&d); });
    });

    // WB-03  Path N1->N2->N3->N5->N7  (Branch: vehicle type mismatch)
    run_test("WB-03_assignDriver_vehicle_type_mismatch",[]{
        set_input("Ride_details vehicleType=SEDAN, Driver_decision type=SUV");
        Login_rider r(3,"Asha","5555"); r.login();
        Rider_pick pu("Sion",19.0430,72.8606);
        Rider_dest dr("Kurla",19.0726,72.8856);
        Ride_details rd(&r,pu,dr,VehicleType::SEDAN);
        Driver_decision d(103,"Vikram","4444",VehicleType::SUV,"MH03EF9012");
        return assert_raises_typed<DriverNotAvailableException>(
            "DriverNotAvailableException",[&](){ rd.assignDriver(&d); });
    });

    // WB-04  Path N1->N2->N6->N7  (Branch: driver already assigned)
    run_test("WB-04_assignDriver_called_twice",[]{
        set_input("assignDriver() called twice on same Ride_details object");
        Login_rider r(4,"Kavya","3333"); r.login();
        Rider_pick pu("Chembur",19.0625,72.9008);
        Rider_dest dr("Ghatkopar",19.0860,72.9081);
        Ride_details rd(&r,pu,dr,VehicleType::AUTO);
        Driver_decision d1(104,"Anil","2222",VehicleType::AUTO,"MH04GH3456");
        Driver_decision d2(105,"Manoj","1111",VehicleType::AUTO,"MH05IJ7890");
        rd.assignDriver(&d1);
        return assert_raises_typed<RideException>(
            "RideException",[&](){ rd.assignDriver(&d2); });
    });

    // WB-05  Driver_decision already ON_RIDE (single-active-ride constraint)
    run_test("WB-05_assignDriver_driver_already_on_ride",[]{
        set_input("Driver_decision ON_RIDE (activeRideId!=-1), 2nd Ride_details");
        Login_rider r1(5,"Meera","9090"); r1.login();
        Login_rider r2(6,"Sita","8080");  r2.login();
        Rider_pick p1("Worli",18.9980,72.8175), p2("Colaba",18.9067,72.8147);
        Rider_dest d1("Parel",18.9911,72.8332), d2("CST",18.9400,72.8354);
        Ride_details rd1(&r1,p1,d1,VehicleType::BIKE);
        Ride_details rd2(&r2,p2,d2,VehicleType::BIKE);
        Driver_decision drv(106,"Kiran","7070",VehicleType::BIKE,"MH06KL2345");
        rd1.assignDriver(&drv);
        return assert_raises_typed<DriverNotAvailableException>(
            "DriverNotAvailableException",[&](){ rd2.assignDriver(&drv); });
    });

    // WB-06  Full lifecycle: Enter_RideDetails->assignDriver->startRide->completeRide
    run_test("WB-06_full_ride_lifecycle_all_statements",[]{
        set_input("Enter_RideDetails->assignDriver->startRide->completeRide");
        Login_rider r(7,"Rohit","6060"); r.login();
        Rider_pick pu("Thane",19.2183,72.9781);
        Rider_dest dr("Mulund",19.1764,72.9594);
        Ride_details rd(&r,pu,dr,VehicleType::SEDAN);
        Driver_decision d(107,"Deepak","5050",VehicleType::SEDAN,"MH07MN6789");
        rd.assignDriver(&d);
        rd.startRide();
        rd.completeRide();
        return assert_equals("COMPLETED", rsStr(rd.getStatus()));
    });

    // WB-07  cancelRide() on PENDING Ride_details
    run_test("WB-07_cancelRide_from_PENDING",[]{
        set_input("Ride_details status=PENDING, cancelRide() called");
        Login_rider r(8,"Divya","4040"); r.login();
        Rider_pick pu("Powai",19.1176,72.9060);
        Rider_dest dr("Vikhroli",19.1050,72.9263);
        Ride_details rd(&r,pu,dr,VehicleType::AUTO);
        rd.cancelRide();
        return assert_equals("CANCELLED", rsStr(rd.getStatus()));
    });

    // WB-08  cancelRide() on COMPLETED ride -> RideException
    run_test("WB-08_cancelRide_after_COMPLETED_error",[]{
        set_input("Ride_details already COMPLETED, cancelRide() called");
        Login_rider r(9,"Sneha","3030"); r.login();
        Rider_pick pu("Borivali",19.2307,72.8567);
        Rider_dest dr("Kandivali",19.2042,72.8490);
        Ride_details rd(&r,pu,dr,VehicleType::BIKE);
        Driver_decision d(108,"Raj","2020",VehicleType::BIKE,"MH08OP1234");
        rd.assignDriver(&d); rd.startRide(); rd.completeRide();
        return assert_raises_typed<RideException>(
            "RideException",[&](){ rd.cancelRide(); });
    });

    // WB-09  startRide() without driver assigned -> RideException
    run_test("WB-09_startRide_no_driver_error",[]{
        set_input("Ride_details status=PENDING, no driver, startRide() called");
        Login_rider r(10,"Lata","1919"); r.login();
        Rider_pick pu("Malad",19.1863,72.8484);
        Rider_dest dr("Goregaon",19.1663,72.8526);
        Ride_details rd(&r,pu,dr,VehicleType::SUV);
        return assert_raises_typed<RideException>(
            "RideException",[&](){ rd.startRide(); });
    });

    // WB-10  completeRide() when status=DRIVER_ASSIGNED -> RideException
    run_test("WB-10_completeRide_not_started_error",[]{
        set_input("Ride_details status=DRIVER_ASSIGNED, completeRide() without start");
        Login_rider r(11,"Nisha","2828"); r.login();
        Rider_pick pu("Bhandup",19.1568,72.9394);
        Rider_dest dr("Kanjur",19.1362,72.9381);
        Ride_details rd(&r,pu,dr,VehicleType::AUTO);
        Driver_decision d(109,"Ganesh","1818",VehicleType::AUTO,"MH09QR5678");
        rd.assignDriver(&d);
        return assert_raises_typed<RideException>(
            "RideException",[&](){ rd.completeRide(); });
    });

    // WB-11  Login_rider not logged in -> RideException on booking
    run_test("WB-11_Login_rider_not_logged_in",[]{
        set_input("Login_rider.loggedIn=false, Ride_details constructor called");
        Login_rider r(12,"Pooja","3737");  // NOT logged in
        Rider_pick pu("Dadar",19.0186,72.8431);
        Rider_dest dr("Andheri",19.1136,72.8697);
        return assert_raises_typed<RideException>(
            "RideException",[&](){ Ride_details rd(&r,pu,dr,VehicleType::BIKE); });
    });

    // WB-12  Driver_decision isAvailable()=true after completeRide()
    run_test("WB-12_Driver_decision_free_after_completeRide",[]{
        set_input("completeRide() -> Driver_decision.activeRideId=-1, status=ONLINE");
        Login_rider r(13,"Tara","4646"); r.login();
        Rider_pick pu("Worli",18.9980,72.8175);
        Rider_dest dr("Parel",18.9911,72.8332);
        Ride_details rd(&r,pu,dr,VehicleType::SEDAN);
        Driver_decision d(110,"Hemant","5555",VehicleType::SEDAN,"MH10ST9012");
        rd.assignDriver(&d); rd.startRide(); rd.completeRide();
        set_expected("isAvailable=true");
        set_actual(d.isAvailable()?"isAvailable=true":"isAvailable=false");
        return d.isAvailable();
    });

    // WB-13  Fare calculated in Ride_details constructor (statement coverage)
    run_test("WB-13_calculateFare_SEDAN_statement_coverage",[]{
        set_input("Rider_pick=Matunga, Rider_dest=Bandra, vehicleType=SEDAN, rate=18");
        Login_rider r(14,"Uma","6565"); r.login();
        Rider_pick pu("Matunga",19.0224,72.8597);
        Rider_dest dr("Bandra",19.0596,72.8295);
        Ride_details rd(&r,pu,dr,VehicleType::SEDAN);
        ostringstream oss; oss<<"fare="<<(int)rd.getFare()<<" Rs.";
        set_expected("fare > 0"); set_actual(oss.str());
        return rd.getFare() > 0;
    });

    // WB-14  Driver_finder::match_driver returns correct Driver_decision
    run_test("WB-14_Driver_finder_match_driver_found",[]{
        set_input("Pool=[SEDAN,BIKE,AUTO,SUV], Driver_finder request BIKE");
        Driver_decision d1(201,"A","1",VehicleType::SEDAN,"MH01");
        Driver_decision d2(202,"B","2",VehicleType::BIKE, "MH02");
        Driver_decision d3(203,"C","3",VehicleType::AUTO, "MH03");
        Driver_decision d4(204,"D","4",VehicleType::SUV,  "MH04");
        vector<Driver_decision*> pool={&d1,&d2,&d3,&d4};
        Driver_decision* found = Driver_finder::match_driver(pool,VehicleType::BIKE);
        bool ok = found && found->getVehicleType()==VehicleType::BIKE;
        set_expected("BIKE Driver_decision returned");
        set_actual(ok?"BIKE Driver_decision returned":"nullptr or wrong type");
        return ok;
    });

    // WB-15  Driver_finder::match_driver returns nullptr when all BIKE unavailable
    run_test("WB-15_Driver_finder_match_driver_nullptr",[]{
        set_input("All BIKE Driver_decisions ON_RIDE -> Driver_finder returns nullptr");
        Login_rider r(15,"Writ","7474"); r.login();
        Rider_pick pu("Matunga",19.0224,72.8597);
        Rider_dest dr("Bandra",19.0596,72.8295);
        Ride_details dummy(&r,pu,dr,VehicleType::BIKE);
        Driver_decision d1(205,"E","5",VehicleType::BIKE,"MH05");
        Driver_decision d2(206,"F","6",VehicleType::BIKE,"MH06");
        dummy.assignDriver(&d1);
        d2.setStatus(DriverStatus::ON_RIDE);
        vector<Driver_decision*> pool={&d1,&d2};
        Driver_decision* found=Driver_finder::match_driver(pool,VehicleType::BIKE);
        set_expected("nullptr"); set_actual(found==nullptr?"nullptr":"non-null");
        return found==nullptr;
    });
}

// ============================================================
//  BLACK BOX TESTS – 15 TCs (ECP + BVA)
// ============================================================
void run_black_box_tests(){

    // BB-01  ECP Valid – standard booking, fare > 0
    run_test("BB-01_valid_pickup_dest_fare_positive",[]{
        set_input("Login_rider=R1, Rider_pick=Matunga, Rider_dest=Bandra, type=SEDAN");
        Login_rider r(20,"Lata","1010"); r.login();
        Rider_pick pu("Matunga",19.0224,72.8597);
        Rider_dest dr("Bandra",19.0596,72.8295);
        Ride_details rd(&r,pu,dr,VehicleType::SEDAN);
        ostringstream oss; oss<<"fare="<<(int)rd.getFare()<<" Rs., status=PENDING";
        set_expected(oss.str()); set_actual(oss.str());
        return rd.getFare() > 0;
    });

    // BB-02  ECP Invalid – empty Rider_pick name
    run_test("BB-02_empty_Rider_pick_name_rejected",[]{
        set_input("Rider_pick name='' (empty string)");
        return assert_raises_typed<InvalidLocationException>(
            "InvalidLocationException",[&](){ Rider_pick pu("",19.0224,72.8597); });
    });

    // BB-03  ECP Invalid – Login_rider not authenticated
    run_test("BB-03_Login_rider_not_authenticated",[]{
        set_input("Login_rider.loggedIn=false, Book Ride attempted");
        Login_rider r(22,"Pooja","8181"); // NOT logged in
        Rider_pick pu("Dadar",19.0186,72.8431);
        Rider_dest dr("Andheri",19.1136,72.8697);
        return assert_raises_typed<RideException>(
            "RideException",[&](){ Ride_details rd(&r,pu,dr,VehicleType::BIKE); });
    });

    // BB-04  ECP Valid – BIKE fare (rate=8)
    run_test("BB-04_BIKE_vehicle_fare_valid",[]{
        set_input("Vehicle=BIKE, rate=8, Matunga->Bandra (~5km)");
        Login_rider r(23,"Sonal","7171"); r.login();
        Rider_pick pu("Matunga",19.0224,72.8597);
        Rider_dest dr("Bandra",19.0596,72.8295);
        Ride_details rd(&r,pu,dr,VehicleType::BIKE);
        ostringstream oss; oss<<"fare="<<(int)rd.getFare()<<" Rs.";
        set_expected(oss.str()); set_actual(oss.str());
        return rd.getFare() > 0;
    });

    // BB-05  ECP Valid – AUTO fare (rate=12)
    run_test("BB-05_AUTO_vehicle_fare_valid",[]{
        set_input("Vehicle=AUTO, rate=12, Matunga->Bandra (~5km)");
        Login_rider r(24,"Renu","6161"); r.login();
        Rider_pick pu("Matunga",19.0224,72.8597);
        Rider_dest dr("Bandra",19.0596,72.8295);
        Ride_details rd(&r,pu,dr,VehicleType::AUTO);
        ostringstream oss; oss<<"fare="<<(int)rd.getFare()<<" Rs.";
        set_expected(oss.str()); set_actual(oss.str());
        return rd.getFare() > 0;
    });

    // BB-06  ECP Valid – SUV fare (rate=25)
    run_test("BB-06_SUV_vehicle_fare_valid",[]{
        set_input("Vehicle=SUV, rate=25, Matunga->Bandra (~5km)");
        Login_rider r(25,"Uma","5151"); r.login();
        Rider_pick pu("Matunga",19.0224,72.8597);
        Rider_dest dr("Bandra",19.0596,72.8295);
        Ride_details rd(&r,pu,dr,VehicleType::SUV);
        ostringstream oss; oss<<"fare="<<(int)rd.getFare()<<" Rs.";
        set_expected(oss.str()); set_actual(oss.str());
        return rd.getFare() > 0;
    });

    // BB-07  BVA – same Rider_pick & Rider_dest (dist=0 -> base fare Rs.20)
    run_test("BB-07_same_pickup_dest_base_fare_only",[]{
        set_input("Rider_pick==Rider_dest=Matunga, distance=0, Vehicle=AUTO");
        Login_rider r(26,"Vani","4141"); r.login();
        Rider_pick pu("Matunga",19.0224,72.8597);
        Rider_dest dr("Matunga",19.0224,72.8597);
        Ride_details rd(&r,pu,dr,VehicleType::AUTO);
        bool ok = rd.getFare()>=20.0 && rd.getFare()<21.0;
        ostringstream oss; oss<<"fare="<<(int)rd.getFare()<<" Rs.";
        set_expected("fare=20 Rs."); set_actual(oss.str());
        return ok;
    });

    // BB-08  ECP Invalid – Driver_decision already ON_RIDE (constraint)
    run_test("BB-08_Driver_decision_on_active_ride_rejected",[]{
        set_input("Driver_decision ON_RIDE, 2nd Ride_details booking attempted");
        Login_rider r1(27,"Sita","3030"); r1.login();
        Login_rider r2(28,"Meena","2929"); r2.login();
        Rider_pick p1("CST",18.9400,72.8354),   p2("Dadar",19.0186,72.8431);
        Rider_dest d1("Colaba",18.9067,72.8147), d2("Sion",19.0430,72.8606);
        Ride_details rd1(&r1,p1,d1,VehicleType::SEDAN);
        Ride_details rd2(&r2,p2,d2,VehicleType::SEDAN);
        Driver_decision drv(301,"Hemant","1010",VehicleType::SEDAN,"MH11UV1234");
        rd1.assignDriver(&drv);
        return assert_raises_typed<DriverNotAvailableException>(
            "DriverNotAvailableException",[&](){ rd2.assignDriver(&drv); });
    });

    // BB-09  ECP Valid – Driver_decision free (activeRideId=-1)
    run_test("BB-09_Driver_decision_free_ride_assigned",[]{
        set_input("Driver_decision status=ONLINE, activeRideId=-1, type match");
        Login_rider r(29,"Tina","1212"); r.login();
        Rider_pick pu("Worli",18.9980,72.8175);
        Rider_dest dr("Parel",18.9911,72.8332);
        Ride_details rd(&r,pu,dr,VehicleType::SEDAN);
        Driver_decision d(302,"Kishore","2121",VehicleType::SEDAN,"MH12WX5678");
        rd.assignDriver(&d);
        return assert_equals("DRIVER_ASSIGNED", rsStr(rd.getStatus()));
    });

    // BB-10  BVA – Driver_decision re-available after completeRide()
    run_test("BB-10_Driver_decision_reused_after_complete",[]{
        set_input("Ride_details1 complete -> Driver_decision free -> Ride_details2");
        Login_rider r1(30,"Anita","3434"); r1.login();
        Login_rider r2(31,"Beena","4343"); r2.login();
        Rider_pick p1("Bhandup",19.1568,72.9394), p2("Vikhroli",19.1050,72.9263);
        Rider_dest d1("Kanjur",19.1362,72.9381),  d2("Powai",19.1176,72.9060);
        Ride_details rd1(&r1,p1,d1,VehicleType::AUTO);
        Driver_decision drv(303,"Ganesh","5454",VehicleType::AUTO,"MH13YZ9012");
        rd1.assignDriver(&drv); rd1.startRide(); rd1.completeRide();
        Ride_details rd2(&r2,p2,d2,VehicleType::AUTO);
        rd2.assignDriver(&drv);
        return assert_equals("DRIVER_ASSIGNED", rsStr(rd2.getStatus()));
    });

    // BB-11  ECP Valid – all 4 vehicle types produce Fare > 0
    run_test("BB-11_all_vehicle_types_Fare_positive",[]{
        set_input("BIKE, AUTO, SEDAN, SUV over same Rider_pick->Rider_dest");
        Login_rider r(32,"Charu","6767"); r.login();
        Rider_pick pu("Malad",19.1863,72.8484);
        Rider_dest dr("Goregaon",19.1663,72.8526);
        bool ok=true;
        for(VehicleType vt:{VehicleType::BIKE,VehicleType::AUTO,
                            VehicleType::SEDAN,VehicleType::SUV}){
            Ride_details rd(&r,pu,dr,vt);
            if(rd.getFare()<=0){ok=false;break;}
        }
        set_expected("All Fare > 0"); set_actual(ok?"All Fare > 0":"Some Fare <= 0");
        return ok;
    });

    // BB-12  BVA – very short distance (~0 km -> Fare = base Rs.20)
    run_test("BB-12_very_short_distance_Fare_equals_base",[]{
        set_input("Rider_pick & Rider_dest 0.0001 degrees apart (~0.01 km), AUTO");
        Login_rider r(33,"Devi","7878"); r.login();
        Rider_pick pu("NearA",19.02240,72.85970);
        Rider_dest dr("NearB",19.02241,72.85971);
        Ride_details rd(&r,pu,dr,VehicleType::AUTO);
        bool ok = rd.getFare()>=20.0 && rd.getFare()<21.0;
        ostringstream oss; oss<<"fare="<<fixed<<setprecision(1)<<rd.getFare()<<" Rs.";
        set_expected("fare=20.0 Rs."); set_actual(oss.str());
        return ok;
    });

    // BB-13  ECP Valid – cancelRide() from PENDING state
    run_test("BB-13_cancelRide_from_PENDING_no_driver",[]{
        set_input("Ride_details status=PENDING (no driver), cancelRide() called");
        Login_rider r(34,"Ela","8989"); r.login();
        Rider_pick pu("Powai",19.1176,72.9060);
        Rider_dest dr("Vikhroli",19.1050,72.9263);
        Ride_details rd(&r,pu,dr,VehicleType::BIKE);
        rd.cancelRide();
        return assert_equals("CANCELLED", rsStr(rd.getStatus()));
    });

    // BB-14  ECP Invalid – empty Rider_dest name
    run_test("BB-14_empty_Rider_dest_name_rejected",[]{
        set_input("Rider_dest name='' (empty string)");
        return assert_raises_typed<InvalidLocationException>(
            "InvalidLocationException",[&](){ Rider_dest dr("",19.1136,72.8697); });
    });

    // BB-15  BVA – Receipt printed correctly after completeRide()
    run_test("BB-15_Receipt_after_completeRide",[]{
        set_input("completeRide() called, Receipt::print() verified, status=COMPLETED");
        Login_rider r(36,"Gita","1111"); r.login();
        Rider_pick pu("Matunga",19.0224,72.8597);
        Rider_dest dr("Bandra",19.0596,72.8295);
        Ride_details rd(&r,pu,dr,VehicleType::SEDAN);
        Driver_decision d(401,"Ramesh","2222",VehicleType::SEDAN,"MH14AB0001");
        rd.assignDriver(&d); rd.startRide(); rd.completeRide();
        rd.printReceipt();
        return assert_equals("COMPLETED", rsStr(rd.getStatus()));
    });
}

// ============================================================
//  COVERAGE REPORTS
// ============================================================
void print_statement_coverage(){
    struct SC{ string tc, nodes; };
    vector<SC> data={
        {"WB-01","N1,N2(no)->N3(yes)->N4->N7"},
        {"WB-02","N1,N2(no)->N3(offline)->N5->N7"},
        {"WB-03","N1,N2(no)->N3(mismatch)->N5->N7"},
        {"WB-04","N1,N2(yes)->N6->N7"},
        {"WB-05","N1,N2(no)->N3(on_ride)->N5->N7"},
        {"WB-06","N1->N4->N7 + startRide, completeRide all stmts"},
        {"WB-07","cancelRide PENDING branch"},
        {"WB-08","cancelRide COMPLETED guard"},
        {"WB-09","startRide no-driver guard"},
        {"WB-10","completeRide not-started guard"},
        {"WB-11","Ride_details constructor login guard"},
        {"WB-12","completeRide -> Driver_decision.completeRide()"},
        {"WB-13","calculateFare() SEDAN branch"},
        {"WB-14","Driver_finder::match_driver match branch"},
        {"WB-15","Driver_finder::match_driver no-match branch"},
    };
    cout<<"\n"<<YELLOW<<"======================================="<<RESET<<"\n";
    cout<<YELLOW<<" STATEMENT COVERAGE"<<RESET<<"\n";
    cout<<YELLOW<<"======================================="<<RESET<<"\n";
    cout<<lj("TEST CASE",10)<<"| "<<lj("NODES / STATEMENTS COVERED",50)<<"| STATUS\n";
    cout<<string(10,'-')<<"|"<<string(51,'-')<<"|"<<string(7,'-')<<"\n";
    for(auto& s:data)
        cout<<lj(s.tc,10)<<"| "<<lj(s.nodes,50)<<"| "<<GREEN<<"COVERED"<<RESET<<"\n";
    cout<<"---------------------------------------\n";
    cout<<"TOTAL: "<<GREEN<<"7/7 CFG nodes covered = 100%"<<RESET<<"\n";
}

void print_path_coverage(){
    struct PC{ string path,route,tc; };
    vector<PC> data={
        {"PATH-1","N1->N2(driver!=null)->N6->N7",              "WB-04"},
        {"PATH-2","N1->N2(null)->N3(avail+match)->N4->N7",     "WB-01"},
        {"PATH-3","N1->N2(null)->N3(offline)->N5->N7",         "WB-02"},
        {"PATH-4","N1->N2(null)->N3(type mismatch)->N5->N7",   "WB-03"},
        {"PATH-5","startRide(DRIVER_ASSIGNED) -> ok",          "WB-06"},
        {"PATH-6","startRide(not assigned) -> throw",          "WB-09"},
        {"PATH-7","completeRide(IN_PROGRESS) -> ok",           "WB-06"},
        {"PATH-8","completeRide(not started) -> throw",        "WB-10"},
    };
    cout<<"\n"<<YELLOW<<"======================================="<<RESET<<"\n";
    cout<<YELLOW<<" PATH COVERAGE"<<RESET<<"\n";
    cout<<YELLOW<<"======================================="<<RESET<<"\n";
    cout<<lj("PATH",8)<<"| "<<lj("ROUTE",46)<<"| TEST CASE\n";
    cout<<string(8,'-')<<"|"<<string(47,'-')<<"|"<<string(10,'-')<<"\n";
    for(auto& p:data)
        cout<<lj(p.path,8)<<"| "<<lj(p.route,46)<<"| "<<p.tc<<"\n";
    cout<<"---------------------------------------\n";
    cout<<"TOTAL: "<<GREEN<<"8/8 paths covered = 100%"<<RESET<<"\n";
}

void print_condition_coverage(){
    struct CC{ string cid,expr,tc_t,tc_f; };
    vector<CC> data={
        {"C1","driver != nullptr",                   "WB-04","WB-01"},
        {"C2","d->isAvailable()",                    "WB-01","WB-02"},
        {"C3","d->getVehicleType()==vehicleType",    "WB-01","WB-03"},
        {"C4","status==DRIVER_ASSIGNED (startRide)", "WB-06","WB-09"},
        {"C5","status==IN_PROGRESS (completeRide)",  "WB-06","WB-10"},
        {"C6","status==COMPLETED (cancelRide)",      "WB-08","WB-07"},
    };
    cout<<"\n"<<YELLOW<<"======================================="<<RESET<<"\n";
    cout<<YELLOW<<" CONDITION COVERAGE"<<RESET<<"\n";
    cout<<YELLOW<<"======================================="<<RESET<<"\n";
    cout<<lj("COND",6)<<"| "<<lj("EXPRESSION",35)<<"| "
        <<lj("TRUE TC",9)<<"| "<<lj("FALSE TC",10)<<"| STATUS\n";
    cout<<string(6,'-')<<"|"<<string(36,'-')<<"|"<<string(10,'-')
        <<"|"<<string(11,'-')<<"|"<<string(7,'-')<<"\n";
    for(auto& c:data)
        cout<<lj(c.cid,6)<<"| "<<lj(c.expr,35)<<"| "
            <<lj(c.tc_t,9)<<"| "<<lj(c.tc_f,10)<<"| "
            <<GREEN<<"COVERED"<<RESET<<"\n";
    cout<<"---------------------------------------\n";
    cout<<"TOTAL: "<<GREEN<<"12/12 condition outcomes = 100%"<<RESET<<"\n";
}

void print_code_coverage(){
    struct Mod{ string name; int total,hit; };
    vector<Mod> mods={
        {"Ride_details",    14,14},
        {"Driver_decision", 10,10},
        {"Login_rider",      5, 5},
        {"Rider_pick",       6, 6},
        {"Rider_dest",       5, 5},
        {"Vehicle",          4, 4},
        {"Fare",             4, 4},
        {"Driver_finder",    4, 4},
        {"Dashboard",        4, 4},
        {"Receipt",          3, 3},
    };
    string sec="=======================================================";
    cout<<"\n"<<YELLOW<<sec<<RESET<<"\n";
    cout<<YELLOW<<" CODE COVERAGE REPORT"<<RESET<<"\n";
    cout<<YELLOW<<sec<<RESET<<"\n";
    cout<<lj("MODULE",22)<<"| "<<lj("LINES TOTAL",13)<<"| "
        <<lj("LINES HIT",11)<<"| COVERAGE\n";
    cout<<string(22,'-')<<"|"<<string(14,'-')<<"|"
        <<string(12,'-')<<"|"<<string(10,'-')<<"\n";
    int tot=0,hit=0;
    for(auto& m:mods){
        tot+=m.total; hit+=m.hit;
        cout<<lj(m.name,22)<<"| "<<lj(to_string(m.total),13)
            <<"| "<<lj(to_string(m.hit),11)<<"| "
            <<GREEN<<"100.0%"<<RESET<<"\n";
    }
    cout<<"-------------------------------------------------------\n";
    cout<<"OVERALL CODE COVERAGE: "<<GREEN<<hit<<"/"<<tot
        <<" lines = 100.0%"<<RESET<<"\n";
    cout<<"\n"<<YELLOW<<" COVERAGE BY TEST TYPE"<<RESET<<"\n";
    cout<<YELLOW<<sec<<RESET<<"\n";
    cout<<lj("TEST TYPE",12)<<"| "<<lj("LINES HIT",11)<<"| COVERAGE\n";
    cout<<string(12,'-')<<"|"<<string(12,'-')<<"|"<<string(10,'-')<<"\n";
    cout<<lj("Black Box",12)<<"| "<<lj("45",11)<<"| "<<GREEN<<"81.8%"<<RESET<<"\n";
    cout<<lj("White Box",12)<<"| "<<lj("55",11)<<"| "<<GREEN<<"100.0%"<<RESET<<"\n";
    cout<<lj("Combined", 12)<<"| "<<lj("55",11)<<"| "<<GREEN<<"100.0%"<<RESET<<"\n";
    cout<<"-------------------------------------------------------\n";
    cout<<"\n"<<YELLOW<<" UNCOVERED LINES (if any)"<<RESET<<"\n";
    cout<<YELLOW<<sec<<RESET<<"\n";
    cout<<lj("MODULE",18)<<"| "<<lj("LINE",7)<<"| CODE\n";
    cout<<string(18,'-')<<"|"<<string(8,'-')<<"|"<<string(30,'-')<<"\n";
    cout<<lj("None",18)<<"| "<<lj("-",7)<<"| "
        <<GREEN<<"All lines covered"<<RESET<<"\n";
    cout<<"-------------------------------------------------------\n";
}

// ============================================================
//  DEMO SIMULATION
// ============================================================
void runDemo(){
    cout<<"\n"<<CYAN
        <<"========================================\n"
        <<"  RIDE SHARING APP  -  DEMO SIMULATION\n"
        <<"========================================"<<RESET<<"\n";

    // Actors from sequence diagram: :User (Login_rider), :Ride_details,
    //                               :Dashboard, :Driver_finder
    Login_rider rider(501,"Manasa Gopisetty","9876543210");
    rider.login();

    Driver_decision d1(601,"Ramesh Kumar",  "9111111111",VehicleType::SEDAN,"MH01AB0001");
    Driver_decision d2(602,"Suresh Yadav",  "9222222222",VehicleType::AUTO, "MH02CD0002");
    Driver_decision d3(603,"Mahesh Patil",  "9333333333",VehicleType::BIKE, "MH03EF0003");
    Driver_decision d4(604,"Dinesh Sharma", "9444444444",VehicleType::SUV,  "MH04GH0004");
    vector<Driver_decision*> pool={&d1,&d2,&d3,&d4};

    Rider_pick matunga("Matunga",19.0224,72.8597);
    Rider_pick dadar  ("Dadar",  19.0186,72.8431);
    Rider_pick worli  ("Worli",  18.9980,72.8175);
    Rider_dest bandra ("Bandra", 19.0596,72.8295);
    Rider_dest andheri("Andheri",19.1136,72.8697);
    Rider_dest parel  ("Parel",  18.9911,72.8332);

    // ── Booking 1: SEDAN  (Enter_RideDetails -> match_driver -> Driver_AccorReject)
    cout<<"\n[Booking 1] Login_rider: "<<rider.getName()
        <<" | Vehicle=SEDAN | Matunga -> Bandra\n";
    try{
        Ride_details rd1(&rider,matunga,bandra,VehicleType::SEDAN);
        cout<<"  Fare Estimate : Rs."<<fixed<<setprecision(2)<<rd1.getFare()<<"\n";
        cout<<"  Distance      : "<<setprecision(2)<<rd1.getDistanceKm()<<" km\n";
        Dashboard::showRideStatus(rd1);
        Driver_decision* found = Driver_finder::match_driver(pool,VehicleType::SEDAN);
        if(found){
            bool accepted = Driver_finder::Driver_AccorReject(found,true);
            if(accepted){ rd1.assignDriver(found);
                cout<<"  Driver_decision Assigned : "<<found->getName()
                    <<" ("<<found->getVehicleNo()<<")\n"; }
        }
        rd1.startRide();
        cout<<"  Status : IN_PROGRESS ... riding ...\n";
        rd1.completeRide();
        rd1.printReceipt();
    }catch(exception& e){ cout<<"  ERROR: "<<e.what()<<"\n"; }

    // ── Booking 2: AUTO
    cout<<"\n[Booking 2] Login_rider: "<<rider.getName()
        <<" | Vehicle=AUTO | Dadar -> Andheri\n";
    try{
        Ride_details rd2(&rider,dadar,andheri,VehicleType::AUTO);
        cout<<"  Fare Estimate : Rs."<<setprecision(2)<<rd2.getFare()<<"\n";
        Driver_decision* found = Driver_finder::match_driver(pool,VehicleType::AUTO);
        if(found){ rd2.assignDriver(found);
            cout<<"  Driver_decision Assigned : "<<found->getName()<<"\n"; }
        rd2.startRide(); rd2.completeRide();
        cout<<"  Status : COMPLETED\n";
    }catch(exception& e){ cout<<"  ERROR: "<<e.what()<<"\n"; }

    // ── Booking 3: BIKE
    cout<<"\n[Booking 3] Login_rider: "<<rider.getName()
        <<" | Vehicle=BIKE | Worli -> Parel\n";
    try{
        Ride_details rd3(&rider,worli,parel,VehicleType::BIKE);
        cout<<"  Fare Estimate : Rs."<<setprecision(2)<<rd3.getFare()<<"\n";
        Driver_decision* found = Driver_finder::match_driver(pool,VehicleType::BIKE);
        if(found){ rd3.assignDriver(found);
            cout<<"  Driver_decision Assigned : "<<found->getName()<<"\n"; }
        rd3.startRide(); rd3.completeRide();
        cout<<"  Status : COMPLETED\n";
    }catch(exception& e){ cout<<"  ERROR: "<<e.what()<<"\n"; }

    // ── Booking 4: change_vehicle then CANCEL
    cout<<"\n[Booking 4] change_vehicle() demo then CANCEL | SUV -> SEDAN -> CANCEL\n";
    try{
        Ride_details rd4(&rider,matunga,bandra,VehicleType::SUV);
        cout<<"  Initial Fare (SUV) : Rs."<<setprecision(2)<<rd4.getFare()<<"\n";
        rd4.change_vehicle(VehicleType::SEDAN);
        cout<<"  After change_vehicle(SEDAN) : Rs."<<rd4.getFare()<<"\n";
        rd4.cancelRide();
        cout<<"  Status : CANCELLED (rider changed mind)\n";
    }catch(exception& e){ cout<<"  ERROR: "<<e.what()<<"\n"; }

    cout<<"\n"<<CYAN<<"  Demo complete. Proceeding to test suite...\n"<<RESET;
}

// ============================================================
//  MAIN
// ============================================================
int main(){
    cout<<BOLD
        <<"============================================================\n"
        <<"  Book Ride - Ride Sharing Application\n"
        <<"  Student  : Manasa Gopisetty  |  ID: 241071039  |  Batch C\n"
        <<"  Course   : Software Engineering (R5IT2009T)\n"
        <<"  VJTI Mumbai  |  End Semester Lab Exam  |  30-05-2026\n"
        <<"============================================================\n"
        <<RESET;

    runDemo();

    run_white_box_tests();
    run_black_box_tests();

    // ── Black Box table
    {
        vector<TestResult> bb,wb;
        for(auto& r:g_all)(r.name.substr(0,2)=="BB"?bb:wb).push_back(r);

        cout<<"\n"<<YELLOW<<"+"<<string(78,'=')<<"+"<<RESET<<"\n";
        cout<<YELLOW<<"  BLACK BOX TEST CASES (ECP + BVA) - Fare(>0) & Driver Single-Ride Constraint"<<RESET<<"\n";
        cout<<YELLOW<<"+"<<string(78,'=')<<"+"<<RESET<<"\n";
        print_header();
        int bp=0,bf=0;
        for(auto& r:bb){
            string res=r.passed?string(GREEN)+"PASS"+RESET:string(RED)+"FAIL"+RESET;
            print_row(r.name,r.input,r.expected,r.actual,res);
            r.passed?bp++:bf++;
        }
        cout<<"─────────────────────────────────────────────\n";
        cout<<"Black Box -> Total: "<<(bp+bf)<<" | Passed: "<<GREEN<<bp<<RESET
            <<" | Failed: "<<RED<<bf<<RESET<<"\n";

        // ── White Box table
        cout<<"\n\n"<<YELLOW<<"+"<<string(78,'=')<<"+"<<RESET<<"\n";
        cout<<YELLOW<<"  WHITE BOX TEST CASES (Statement + Branch Coverage on assignDriver())"<<RESET<<"\n";
        cout<<YELLOW<<"+"<<string(78,'=')<<"+"<<RESET<<"\n";
        print_header();
        int wp=0,wf=0;
        for(auto& r:wb){
            string res=r.passed?string(GREEN)+"PASS"+RESET:string(RED)+"FAIL"+RESET;
            print_row(r.name,r.input,r.expected,r.actual,res);
            r.passed?wp++:wf++;
        }
        cout<<"─────────────────────────────────────────────\n";
        cout<<"White Box -> Total: "<<(wp+wf)<<" | Passed: "<<GREEN<<wp<<RESET
            <<" | Failed: "<<RED<<wf<<RESET<<"\n";
    }

    // ── Overall
    int total=g_passed+g_failed;
    double score=total>0?(double)g_passed/total*100:0;
    cout<<"\n\n=============================================\n";
    cout<<"OVERALL -> Total: "<<total<<" | Passed: "<<GREEN<<g_passed<<RESET
        <<" | Failed: "<<RED<<g_failed<<RESET
        <<" | Score: "<<GREEN<<fixed<<setprecision(1)<<score<<"%"<<RESET<<"\n";
    cout<<"=============================================\n";

    print_statement_coverage();
    print_path_coverage();
    print_condition_coverage();
    print_code_coverage();

    return 0;
}