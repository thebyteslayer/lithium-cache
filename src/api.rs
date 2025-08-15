// Copyright (c) 2025, TheByteSlayer, Lithium Cache

use crate::cache::Cache;
use std::sync::Arc;

#[derive(Debug, Clone)]
pub struct ApiHandler {
    cache: Arc<Cache>,
    valid_token: String,
}

impl ApiHandler {
    pub fn new(cache: Arc<Cache>, token: String) -> Self {
        ApiHandler {
            cache,
            valid_token: token,
        }
    }

    pub fn handle_request(&self, request: &str) -> String {
        let request = request.trim();
        
        if let Some((token_part, command_part)) = request.split_once('.') {
            if token_part != self.valid_token {
                return "Request declined: Invalid Token".to_string();
            }

            return self.execute_command(command_part);
        }

        "Invalid request format".to_string()
    }

    fn execute_command(&self, command_part: &str) -> String {
        if command_part == "keys" {
            return self.handle_keys();
        }

        if let Some((command, args_part)) = command_part.split_once('(') {
            if !args_part.ends_with(')') {
                return "Invalid command format".to_string();
            }

            let args_content = &args_part[..args_part.len() - 1];

            match command {
                "set" => self.handle_set(args_content),
                "get" => self.handle_get(args_content),
                "del" => self.handle_del(args_content),
                _ => "Unknown command".to_string(),
            }
        } else {
            "Invalid command format".to_string()
        }
    }

    fn handle_set(&self, args: &str) -> String {
        let parts = self.parse_set_args(args);
        if parts.len() != 2 {
            return "Invalid arguments for set command. Expected: set(\"key\", \"value\")".to_string();
        }

        let key = parts[0].clone();
        let value = parts[1].clone();

        match self.cache.set(key.clone(), value) {
            Ok(()) => format!("OK: Set key '{}'", key),
            Err(e) => format!("Error: {}", e),
        }
    }

    fn handle_get(&self, args: &str) -> String {
        let key = self.parse_quoted_string(args);
        if key.is_empty() {
            return "Invalid arguments for get command. Expected: get(\"key\")".to_string();
        }

        match self.cache.get(&key) {
            Ok(Some(value)) => value,
            Ok(None) => "null".to_string(),
            Err(e) => format!("Error: {}", e),
        }
    }

    fn handle_del(&self, args: &str) -> String {
        let key = self.parse_quoted_string(args);
        if key.is_empty() {
            return "Invalid arguments for del command. Expected: del(\"key\")".to_string();
        }

        match self.cache.delete(&key) {
            Ok(true) => "OK: Key deleted".to_string(),
            Ok(false) => "Key not found".to_string(),
            Err(e) => format!("Error: {}", e),
        }
    }

    fn handle_keys(&self) -> String {
        match self.cache.keys() {
            Ok(keys) => {
                if keys.is_empty() {
                    "[]".to_string()
                } else {
                    format!("[{}]", keys.iter().map(|k| format!("\"{}\"", k)).collect::<Vec<_>>().join(", "))
                }
            }
            Err(e) => format!("Error: {}", e),
        }
    }

    fn parse_set_args(&self, args: &str) -> Vec<String> {
        let mut result = Vec::new();
        let mut current = String::new();
        let mut in_quotes = false;
        let mut escaped = false;
        let mut chars = args.chars().peekable();

        while let Some(ch) = chars.next() {
            if escaped {
                current.push(ch);
                escaped = false;
            } else if ch == '\\' {
                escaped = true;
            } else if ch == '"' {
                in_quotes = !in_quotes;
            } else if ch == ',' && !in_quotes {
                if !current.trim().is_empty() {
                    result.push(current.trim().to_string());
                    current.clear();
                }
            } else if !ch.is_whitespace() || in_quotes {
                current.push(ch);
            }
        }

        if !current.trim().is_empty() {
            result.push(current.trim().to_string());
        }

        result
    }

    fn parse_quoted_string(&self, input: &str) -> String {
        let trimmed = input.trim();
        if trimmed.starts_with('"') && trimmed.ends_with('"') && trimmed.len() >= 2 {
            trimmed[1..trimmed.len() - 1].to_string()
        } else {
            String::new()
        }
    }
}