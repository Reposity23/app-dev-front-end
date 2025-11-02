# Smart Toy Store System - Replit Configuration

## Overview
The Smart Toy Store is a full-stack, real-time order processing and factory management system designed for a toy factory. This project was originally designed to run on a local network with a dedicated WiFi hotspot, but has been adapted to run on Replit.

### Project Components
1. **Flutter Mobile App** - Customer-facing mobile application (in `lib/` directory)
2. **Dart Backend Server** - Central API and WebSocket server (in `backend/` directory)
3. **HTML5 Dashboard** - Real-time factory floor interface (in `dashboard/` directory)
4. **Arduino Integration** - Physical RFID scanner and LED system (in `arduino/` directory, for reference only)

## Architecture
- **Backend Server**: Dart-based server using Shelf framework
  - Port: 5000 (configured for Replit)
  - Serves REST API endpoints
  - Provides WebSocket for real-time updates
  - Serves static dashboard files
  - Persistent JSON database for users and orders

- **Dashboard**: HTML5/JavaScript web interface
  - Real-time order monitoring
  - Audio alerts for new orders
  - Order status tracking
  - Clear history functionality

## Recent Changes

### 2025-11-02 - Replit Environment Setup
- Installed Dart 3.8 SDK
- Configured backend server to run on port 5000 (changed from 8080)
- Set up workflow to run backend server automatically
- Configured autoscale deployment for production
- Dashboard successfully serves on root path (/)

## User Preferences
No specific user preferences documented yet.

## Project Structure
```
.
├── android/           # Android platform files
├── arduino/           # Arduino/ESP8266 RFID scanner code (reference only)
├── assets/            # Images and sound files for mobile app
├── backend/           # Dart backend server
│   ├── bin/
│   │   └── server.dart  # Main server file
│   ├── lib/
│   │   ├── auth.dart
│   │   ├── database.dart
│   │   └── models.dart
│   ├── data/          # JSON database files
│   └── pubspec.yaml
├── dashboard/         # HTML5 real-time dashboard
│   ├── index.html
│   └── sounds/        # Audio notifications
├── ios/               # iOS platform files
├── lib/               # Flutter mobile app source
├── web/               # Flutter web build files
└── pubspec.yaml       # Flutter dependencies
```

## Development Notes

### Running Locally on Replit
The backend server automatically starts via the configured workflow:
- Command: `cd backend && dart run bin/server.dart`
- Access dashboard at the Replit webview URL

### Original Local Network Setup (for reference)
The system was originally designed to run on:
- WiFi SSID: `db`
- Password: `123456789`
- Static IP: `192.168.137.1:8080`
- All devices (mobile app, dashboard, Arduino) connected to this network

### Database
- Uses JSON files for persistent storage
- Located in `backend/data/`
- Files: `users.json`, `orders.json`

### API Endpoints
- POST `/api/login` - User authentication
- POST `/api/signup` - User registration
- POST `/api/orders` - Create new order
- GET `/api/orders` - Get all orders
- POST `/api/orders/clear` - Clear order history
- POST `/api/process-next` - Process next order for a worker
- GET `/ws` - WebSocket connection for real-time updates

### Order Workflow
1. Customer places order via mobile app
2. Backend assigns order to factory worker based on toy category
3. Dashboard displays order in real-time with audio alert
4. Worker scans RFID tag (via Arduino or API)
5. Order progresses: PENDING → PROCESSING → ON_THE_WAY → DELIVERED → COMPLETED

### Worker Assignments (Hardcoded by Category)
- Dolls: Marl
- Puzzles: Renz
- Action Figures: Cruz
- Toy Guns: John Marwin (Ebona)

## Deployment
Configured for Replit Autoscale deployment:
- Runs `dart run backend/bin/server.dart`
- Automatically serves on port 5000
- Stateless deployment with persistent JSON database

## Known Issues
None currently documented.
