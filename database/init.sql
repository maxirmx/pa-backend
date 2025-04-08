-- Secure Application Database Initialization Script

-- Create database (run this manually as superuser)
-- CREATE DATABASE secure_app;
-- CREATE USER app_user WITH PASSWORD 'change_this_password';
-- GRANT ALL PRIVILEGES ON DATABASE secure_app TO app_user;

-- Connect to the database first, then run the following:
-- \c secure_app

-- Drop tables if they exist (for resetting the database)
DROP TABLE IF EXISTS user_tokens;
DROP TABLE IF EXISTS users;
DROP TABLE IF EXISTS audit_log;

-- Create users table
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    password_hash VARCHAR(100) NOT NULL,
    full_name VARCHAR(100),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_login TIMESTAMP,
    is_active BOOLEAN DEFAULT TRUE,
    is_admin BOOLEAN DEFAULT FALSE
);

-- Create user_tokens table (for authentication tokens)
CREATE TABLE user_tokens (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    token VARCHAR(255) NOT NULL,
    expires_at TIMESTAMP NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    is_revoked BOOLEAN DEFAULT FALSE,
    CONSTRAINT user_tokens_token_key UNIQUE (token)
);

-- Create audit_log table for security audit trail
CREATE TABLE audit_log (
    id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES users(id) ON DELETE SET NULL,
    action VARCHAR(50) NOT NULL,
    resource_type VARCHAR(50) NOT NULL,
    resource_id VARCHAR(50),
    details JSONB,
    ip_address VARCHAR(45),
    user_agent VARCHAR(255),
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Create indexes
CREATE INDEX idx_users_username ON users(username);
CREATE INDEX idx_users_email ON users(email);
CREATE INDEX idx_user_tokens_token ON user_tokens(token);
CREATE INDEX idx_user_tokens_user_id ON user_tokens(user_id);
CREATE INDEX idx_audit_log_user_id ON audit_log(user_id);
CREATE INDEX idx_audit_log_action ON audit_log(action);
CREATE INDEX idx_audit_log_timestamp ON audit_log(timestamp);

-- Add trigger to update updated_at timestamp
CREATE OR REPLACE FUNCTION update_timestamp()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER update_users_timestamp
BEFORE UPDATE ON users
FOR EACH ROW
EXECUTE FUNCTION update_timestamp();

-- Create a sample admin user (password: admin123)
-- The password_hash would normally be a bcrypt hash
INSERT INTO users (username, email, password_hash, full_name, is_admin)
VALUES ('admin', 'admin@example.com', '$2a$12$1tGMYqXh0ICYBZgXjmgF8uMee8zcP5yCxEkUrSmw6rNZ8z2r71RBW', 'System Admin', TRUE);

-- Create a sample regular user (password: user123)
INSERT INTO users (username, email, password_hash, full_name)
VALUES ('user', 'user@example.com', '$2a$12$PJ.Z1nRSEkBDLgh03dcb.OFz0LSF5Vd0nCRNhvX6kz3YUqhxRJPZO', 'Regular User');

-- Add initial audit log entry
INSERT INTO audit_log (user_id, action, resource_type, resource_id, details)
VALUES (1, 'SYSTEM_INIT', 'DATABASE', NULL, '{"message": "Database initialized"}');

-- Grant permissions
ALTER TABLE users OWNER TO app_user;
ALTER TABLE user_tokens OWNER TO app_user;
ALTER TABLE audit_log OWNER TO app_user;
