// Copyright (c) 2025, TheByteSlayer, Lithium Cache

use chrono::{DateTime, Local};
use std::fs::{self, File, OpenOptions};
use std::io::{self, Write};
use std::path::Path;
use std::sync::{Arc, Mutex};

pub struct FileLogger {
    log_file: Arc<Mutex<File>>,
    logs_dir: String,
}

impl FileLogger {
    pub fn new() -> io::Result<Self> {
        let logs_dir = "logs";
        
        if !Path::new(logs_dir).exists() {
            fs::create_dir(logs_dir)?;
        }

        Self::handle_existing_logs(logs_dir)?;

        let active_log_path = format!("{}/active.log", logs_dir);
        let log_file = OpenOptions::new()
            .create(true)
            .write(true)
            .truncate(true)
            .open(&active_log_path)?;

        Ok(FileLogger {
            log_file: Arc::new(Mutex::new(log_file)),
            logs_dir: logs_dir.to_string(),
        })
    }

    fn handle_existing_logs(logs_dir: &str) -> io::Result<()> {
        let active_log_path = format!("{}/active.log", logs_dir);
        let latest_log_path = format!("{}/latest.log", logs_dir);

        if Path::new(&latest_log_path).exists() {
            let now: DateTime<Local> = Local::now();
            let timestamp = now.format("%d.%m.%Y-%H:%M:%S").to_string();
            let archived_log_path = format!("{}/{}.log", logs_dir, timestamp);
            
            fs::rename(&latest_log_path, &archived_log_path)?;
        }

        if Path::new(&active_log_path).exists() {
            fs::rename(&active_log_path, &latest_log_path)?;
        }

        Ok(())
    }

    pub fn log(&self, message: &str) {
        let now: DateTime<Local> = Local::now();
        let timestamp = now.format("[%Y-%m-%d %H:%M:%S%.3f %Z]").to_string();
        let log_entry = format!("{} {}\n", timestamp, message);

        if let Ok(mut file) = self.log_file.lock() {
            let _ = file.write_all(log_entry.as_bytes());
            let _ = file.flush();
        }
    }

    pub fn log_request(&self, request: &str) {
        self.log(&format!("INFO {}", request));
    }

    pub fn log_response(&self, response: &str) {
        self.log(&format!("INFO {}", response));
    }

    pub fn log_info(&self, message: &str) {
        self.log(&format!("INFO {}", message));
    }

    pub fn log_error(&self, message: &str) {
        self.log(&format!("ERROR {}", message));
    }

    pub fn shutdown(&self) -> io::Result<()> {
        let active_log_path = format!("{}/active.log", self.logs_dir);
        let latest_log_path = format!("{}/latest.log", self.logs_dir);

        if Path::new(&active_log_path).exists() {
            fs::rename(&active_log_path, &latest_log_path)?;
        }

        Ok(())
    }
}

impl Drop for FileLogger {
    fn drop(&mut self) {
        let _ = self.shutdown();
    }
}
