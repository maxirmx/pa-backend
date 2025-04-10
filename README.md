# Secure Application Server

A secure HTTP/HTTPS server application built with C++, Proxygen, and PostgreSQL.

## Features

- HTTP and HTTPS support
- RESTful API endpoints
- PostgreSQL database connectivity
- JSON Web Token (JWT) authentication
- Rate limiting
- Logging and monitoring

## Requirements

- C++17 compiler
- CMake 3.10 or newer
- Facebook Proxygen library
- PostgreSQL and libpq
- Boost libraries
- OpenSSL
- glog, gflags
- nlohmann_json

## Dependencies Installation

On Ubuntu/Debian:

```bash
sudo apt-get install git cmake g++ libgflags-dev libgoogle-glog-dev libboost-all-dev \
  libevent-dev libdouble-conversion-dev libssl-dev libpq-dev postgresql-client \
  nlohmann-json3-dev
```

## Building

1. Clone the repository
2. Run the build script:

```bash
./build.sh
```

The script will:
- Create the build directory
- Generate CMake files
- Build the application
- Generate self-signed SSL certificates for development (if they don't exist)
- Create logs directory

## Database Setup

1. Make sure PostgreSQL server is running
2. Create database and user:

```sql
CREATE DATABASE secure_app;
CREATE USER app_user WITH PASSWORD 'change_this_password';
GRANT ALL PRIVILEGES ON DATABASE secure_app TO app_user;
```

3. Initialize the database schema:

```bash
psql -U app_user -d secure_app -f database/init.sql
```

## Configuration

The server is configured via a JSON file located at `config/server_config.json`. Key configuration options:

- Server host and ports
- SSL certificate paths
- Database connection parameters
- Security settings including JWT secret
- Logging configuration

## Running the Server

```bash
cd build
./secure_app_server --config=../config/server_config.json --verbose
```

Command line options:
- `--config`: Path to configuration file
- `--verbose`: Enable verbose logging
- `--logdir`: Directory for log files (default: "./logs")

## API Endpoints

### Health Check
- GET `/health` - Check server and database health

### Authentication
- POST `/api/auth` - Authenticate user and get JWT token

### Users
- GET `/api/users` - Get list of users
- POST `/api/users` - Create new user

## Security Features

- HTTPS with strong cipher configuration
- JWT token-based authentication
- Password hashing
- Rate limiting
- Audit logging

## Development

### Project Structure

- `CMakeLists.txt`: Main build configuration
- `config/`: Configuration files
- `database/`: SQL scripts
- `include/`: Header files
  - `db/`: Database management classes
  - `handlers/`: HTTP request handlers
- `src/`: Implementation files
- `ssl/`: SSL certificates

## License

[MIT License](LICENSE)
