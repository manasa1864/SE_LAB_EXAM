# Book Ride — Ride Sharing Application

<div align="center">

![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue?style=flat-square&logo=cplusplus)
![Status](https://img.shields.io/badge/Tests-30%2F30%20Passed-brightgreen?style=flat-square)
![Coverage](https://img.shields.io/badge/Coverage-100%25-brightgreen?style=flat-square)
![Exam](https://img.shields.io/badge/End%20Sem%20Lab%20Exam-30--05--2026-orange?style=flat-square)

</div>

---

<div align="center">

| | |
|---|---|
| **Student** | Manasa Gopisetty |
| **ID** | 241071039 |
| **Batch** | C |
| **Course** | Software Engineering (R5IT2009T) |
| **Institute** | VJTI Mumbai — IV B.Tech Computer Engineering |
| **Date** | 30-04-2026 |

</div>

---

## Table of Contents

- [Problem Statement](#problem-statement)
- [Q1 — Paper Based](#q1--paper-based)
- [Q2 — Implementation](#q2--implementation)
- [Class Model](#class-model)
- [White Box Testing](#white-box-testing)
- [Black Box Testing](#black-box-testing)
- [How to Run](#how-to-run)
- [Test Results](#test-results)
- [Files in this Repository](#files-in-this-repository)

---

## Problem Statement

A mobility startup requires a ride-hailing platform that connects riders and drivers for on-demand transport. The system supports ride booking, real-time tracking, dynamic pricing, driver ratings, and secure payment processing.

The **Book Ride** use case allows a Rider to provide:

- Pickup location (`Rider_pick`)
- Drop / destination location (`Rider_dest`)
- Vehicle type — `BIKE / AUTO / SEDAN / SUV`

The system matches an available driver, calculates fare, and manages the complete ride lifecycle.

---

## Q1 — Paper Based

### 1. Use Case Diagram

**Actors:** Rider · Driver · Admin

| Actor | Use Cases |
|---|---|
| Rider | Login, Book Ride, Select Pickup & Drop, Select Vehicle, View Fare, View Ride History, View Receipt |
| Driver | Accept / Reject Ride, View Earnings, View Trip History, Availability Toggle, Document Management |
| Admin | Driver Onboarding, Driver Verification, Surge Pricing & Zone Management, Dispute Resolution, Customer Support |

> Book Ride `<<includes>>` Login

---

### 2. Use Case Specification — Book Ride

| Field | Details |
|---|---|
| **Use Case ID** | UC-02 |
| **Use Case Name** | Book Ride |
| **Actors** | Rider |
| **Date Created** | 30/04/26 |
| **Created By** | Manasa Gopisetty |
| **Preconditions** | Rider logged in with valid credentials (name, age, password) |
| **Postconditions** | Pickup & destination locations confirmed, driver matched |
| **Priority** | High |
| **Frequency of Use** | High |

**Normal Flow of Events**

1. Login
2. Click the Book Ride button
3. Enter pickup location
4. Enter destination location
5. Select vehicle type
6. View payment details
7. Confirm ride
8. Track driver

**Alternate Flow of Events**

1. Login → Select vehicle type
2. View payment
3. If fare too high due to selected vehicle → go back and choose a different vehicle

**Includes:** Login

---

### 3. Sequence Diagram — Book Ride (with Driver Matching)

```
:User          :Ride_details        :Dashboard         :Driver_finder
  │                  │                    │                   │
  │──Enter_RideDetails()──►│              │                   │
  │◄──change_vehicle()─────│              │                   │
  │                  │────────match_driver()────────────────► │
  │                  │         │           │◄──Driver_AccorReject()──│
  │◄──Driver_Found()──────────────────────────────────────────│
  │                  │                    │                   │
```

---

### 4. CRC Cards

**Entity — `Ride_details`**

| Responsibilities | Collaborations |
|---|---|
| Store login details of rider | `Login_rider` |
| Enter pickup location | `Rider_pick` |
| Enter destination location | `Rider_dest` |
| Enter vehicle type | `Vehicle` |
| Choice of driver | `Driver_decision` |
| Fare and ride history | `Fare`, `Ride_history`, `Receipt` |

**Boundary — `Dashboard`**

| Responsibilities | Collaborations |
|---|---|
| View driver's location | `Driver_loc_rider` |
| View driver's earnings | `Earning_dashboard` |
| View rider's ride history | `Ride_history` |
| View driver's trip history | `Trip_history` |

**Control — `Driver_finder`**

| Responsibilities | Collaborations |
|---|---|
| Get rider's location from pickup, search drivers | `Rider_movement`, `Driver_search_engine` |
| Driver accepts / rejects → match | `Driver_pick_engine` |
| Map leading driver to rider's location | `Driver_map_pick` |
| Driver authenticates rider, picks up to destination | `Driver_auth`, `Driver_map_dest` |

---

## Q2 — Implementation

### A.1 Implementation (10 Marks)

Implemented in **C++17** using OOP principles, based directly on the Q1 class model.

#### Classes

| Class | Type | Responsibility |
|---|---|---|
| `Ride_details` | Entity | Core ride entity — stores all ride data, manages lifecycle |
| `Login_rider` | Entity | Rider authentication — login / logout |
| `Rider_pick` | Entity | Pickup location (name, latitude, longitude, distance calc) |
| `Rider_dest` | Entity | Drop / destination location |
| `Vehicle` | Entity | Vehicle type and registration number |
| `Fare` | Entity | Calculated fare with positive-value validation |
| `Receipt` | Entity | Prints ride receipt after completion |
| `Driver_decision` | Entity | Driver with single-active-ride enforcement |
| `Driver_finder` | Control | `match_driver()`, `Driver_AccorReject()` — matches driver to ride |
| `Dashboard` | Boundary | `showRideStatus()`, `showDriverLocation()` — display layer |

#### Custom Exceptions

| Exception | Thrown When |
|---|---|
| `RideException` | Base exception for all ride errors |
| `InvalidLocationException` | Location name is empty |
| `DriverNotAvailableException` | Driver offline, on ride, or vehicle type mismatch |
| `InvalidFareException` | Calculated fare is ≤ 0 |

#### Fare Calculation

```
Fare  =  (rate per km  ×  distance_km)  +  Rs. 20 base charge

  BIKE   →  Rs.  8 / km
  AUTO   →  Rs. 12 / km
  SEDAN  →  Rs. 18 / km
  SUV    →  Rs. 25 / km
```

---

## Class Model

```
                    ┌──────────────────────────────────────┐
                    │           Ride_details               │
                    │  ─────────────────────────────────   │
                    │  rideId · rideStatus · distanceKm    │
                    │  ─────────────────────────────────   │
                    │  + assignDriver()   ◄── WB TEST      │
                    │  + calculateFare()                   │
                    │  + startRide()                       │
                    │  + completeRide()                    │
                    │  + cancelRide()                      │
                    │  + change_vehicle()                  │
                    │  + match_driver()                    │
                    └──┬──────┬───────┬──────┬────────────┘
                       │      │       │      │
                 Login  │  Rider │  Rider │  Driver
                _rider  │  _pick │  _dest │  _decision
                                               │
                                        Driver_finder  (Control)
                                               │
                                           Dashboard   (Boundary)
```

---

## White Box Testing

### Method Under Test: `Ride_details::assignDriver()`

#### Control Flow Graph

```
  N1  (Entry)
   │
   ▼
  N2  driver != nullptr ?
   │
   ├── YES ──────────────────────► N6  throw RideException
   │                                        │
   └── NO                                   │
        │                                   │
        ▼                                   │
       N3  isAvailable() && typeMatch ?     │
        │                                   │
        ├── YES ──► N4  assign driver       │
        │                │                  │
        └── NO  ──► N5  throw               │
                    DriverNotAvailable       │
                         │                  │
                         └────────┬─────────┘
                                  ▼
                                 N7  (Exit)

  Edges = 8   Nodes = 7   V(G) = E − N + 2P = 8 − 7 + 2 = 3
```

#### Independent Test Paths

| Path | Route | Covered By |
|---|---|---|
| Path 1 | N1 → N2 → N3 → N4 → N7 | WB-01 |
| Path 2 | N1 → N2 → N3 → N5 → N7 | WB-02, WB-03, WB-05 |
| Path 3 | N1 → N2 → N6 → N7 | WB-04 |

#### White Box Test Cases

| TC ID | Type | Description | Expected Output | Result |
|---|---|---|---|---|
| WB-01 | Statement | Available driver, correct vehicle type | DRIVER_ASSIGNED | ✅ PASS |
| WB-02 | Branch | Driver OFFLINE → unavailable | DriverNotAvailableException | ✅ PASS |
| WB-03 | Branch | Vehicle type mismatch (SEDAN vs SUV) | DriverNotAvailableException | ✅ PASS |
| WB-04 | Branch | `assignDriver()` called twice | RideException | ✅ PASS |
| WB-05 | Branch | Driver already ON_RIDE | DriverNotAvailableException | ✅ PASS |
| WB-06 | Statement | Full lifecycle — all statements executed | COMPLETED | ✅ PASS |
| WB-07 | Branch | `cancelRide()` from PENDING state | CANCELLED | ✅ PASS |
| WB-08 | Branch | `cancelRide()` after COMPLETED | RideException | ✅ PASS |
| WB-09 | Branch | `startRide()` without driver assigned | RideException | ✅ PASS |
| WB-10 | Branch | `completeRide()` when not IN_PROGRESS | RideException | ✅ PASS |
| WB-11 | Statement | `Login_rider` not logged in | RideException | ✅ PASS |
| WB-12 | Branch | Driver free after `completeRide()` | isAvailable() = true | ✅ PASS |
| WB-13 | Statement | `calculateFare()` — SEDAN branch | fare > 0 (≈ Rs. 115) | ✅ PASS |
| WB-14 | Statement | `Driver_finder::match_driver` — match found | BIKE driver returned | ✅ PASS |
| WB-15 | Branch | `Driver_finder::match_driver` — no match | nullptr | ✅ PASS |

---

## Black Box Testing

### ECP Partitions

| Input | Valid Partition | Invalid Partition |
|---|---|---|
| Fare | Fare > 0 | Fare ≤ 0 |
| Location name | Non-empty string | Empty string `""` |
| Rider authentication | `loggedIn = true` | `loggedIn = false` |
| Driver status | `activeRideId = -1` | `activeRideId ≠ -1` (ON_RIDE) |

### BVA Boundary Values

| Boundary | Input | Expected Output |
|---|---|---|
| Distance = 0 | Same pickup & drop location | Fare = Rs. 20 (base charge only) |
| Distance ≈ 0 | 0.0001° apart (~0.01 km) | Fare ≈ Rs. 20 |
| Driver just completed ride | `activeRideId` reset to -1 | `isAvailable()` = true |

### Black Box Test Cases

| TC ID | Type | Description | Expected Output | Result |
|---|---|---|---|---|
| BB-01 | ECP Valid | Valid `Rider_pick` & `Rider_dest`, Fare > 0 | fare = 115 Rs., PENDING | ✅ PASS |
| BB-02 | ECP Invalid | Empty `Rider_pick` name | InvalidLocationException | ✅ PASS |
| BB-03 | ECP Invalid | `Login_rider` not authenticated | RideException | ✅ PASS |
| BB-04 | ECP Valid | Vehicle = BIKE (rate = 8) | fare = 62 Rs. | ✅ PASS |
| BB-05 | ECP Valid | Vehicle = AUTO (rate = 12) | fare = 83 Rs. | ✅ PASS |
| BB-06 | ECP Valid | Vehicle = SUV (rate = 25) | fare = 152 Rs. | ✅ PASS |
| BB-07 | BVA Boundary | Same pickup & drop (distance = 0) | fare = Rs. 20 | ✅ PASS |
| BB-08 | ECP Invalid | `Driver_decision` ON_RIDE constraint | DriverNotAvailableException | ✅ PASS |
| BB-09 | ECP Valid | Driver free (`activeRideId = -1`) | DRIVER_ASSIGNED | ✅ PASS |
| BB-10 | BVA Boundary | Driver reuse after `completeRide()` | DRIVER_ASSIGNED | ✅ PASS |
| BB-11 | ECP Valid | All 4 vehicle types produce Fare > 0 | All Fare > 0 | ✅ PASS |
| BB-12 | BVA Boundary | Very short distance (~0.01 km) | fare ≈ Rs. 20 | ✅ PASS |
| BB-13 | ECP Valid | `cancelRide()` from PENDING, no driver | CANCELLED | ✅ PASS |
| BB-14 | ECP Invalid | Empty `Rider_dest` name | InvalidLocationException | ✅ PASS |
| BB-15 | BVA Boundary | `Receipt::print()` after `completeRide()` | COMPLETED, receipt OK | ✅ PASS |

---

## How to Run

```bash
g++ book_ride_final.cpp
./a.out
```

The program runs in three stages:

**Stage 1 — Demo Simulation**
Four live bookings showing the full ride flow — fare estimation, driver matching, ride lifecycle, receipt printing, and a cancel demo.

**Stage 2 — Test Suite**
15 Black Box + 15 White Box test cases printed in a formatted table with colour-coded PASS / FAIL results.

**Stage 3 — Coverage Reports**
Statement coverage, path coverage, condition coverage, and code coverage tables.

---

## Test Results

<div align="center">

| Section | Test Cases | Passed | Failed | Score |
|:---:|:---:|:---:|:---:|:---:|
| White Box | 15 | 15 | 0 | 100% |
| Black Box | 15 | 15 | 0 | 100% |
| **Total** | **30** | **30** | **0** | **100%** |

</div>

| Coverage Metric | Result |
|---|---|
| Statement Coverage | 7 / 7 CFG nodes — 100% |
| Path Coverage | 8 / 8 independent paths — 100% |
| Condition Coverage | 12 / 12 condition outcomes — 100% |
| Code Coverage | 59 / 59 lines — 100% |

---

## Files in this Repository

| File | Description |
|---|---|
| `book_ride_final.cpp` | C++ implementation — demo + 30 test cases + coverage reports |
| `BookRide_Professional.xlsx` | Excel test case sheet — White Box & Black Box (3 sheets) |
| `README.md` | This file |
| `Output1.jpeg` | Terminal — demo simulation |
| `Output2.jpeg` | Terminal — Black Box test results |
| `Output3.jpeg` | Terminal — White Box test results |
| `Output4.jpeg` | Terminal — coverage reports |

---

<div align="center">

*VJTI Mumbai &nbsp;·&nbsp; Software Engineering (R5IT2009T) &nbsp;·&nbsp; End Semester Lab Exam &nbsp;·&nbsp; 30-05-2026*

</div>
# SE_LAB_EXAM
