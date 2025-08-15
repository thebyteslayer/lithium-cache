// Copyright (c) 2025, TheByteSlayer, Lithium Cache

mod config;
mod token;
mod cache;
mod api;
mod logging;

use config::Config;
use token::TokenConfig;
use cache::Cache;
use api::ApiHandler;
use logging::FileLogger;
use log::{info, error, LevelFilter};
use std::io;
use std::sync::Arc;
use tokio::net::TcpListener;
use tokio::signal;

#[tokio::main]
async fn main() -> io::Result<()> {
    env_logger::Builder::from_default_env()
        .filter_level(LevelFilter::Info)
        .init();
    
    let config = Config::load_or_create()?;
    
    let bind_address = format!("{}:{}", config.bind.ip, config.bind.port);
    
    let listener = TcpListener::bind(&bind_address).await?;
    
    let file_logger = if config.logging {
        Some(Arc::new(FileLogger::new()?))
    } else {
        None
    };

    if !config.silent {
        info!("Lithium Cache listening on {}", bind_address);
        if let Some(ref logger) = file_logger {
            logger.log_info(&format!("Lithium Cache listening on {}", bind_address));
        }
    }
    
    let token_config = TokenConfig::load_or_create()?;
    
    if !config.silent {
        info!("Lithium Cache initialized with token: {}", token_config.token);
        if let Some(ref logger) = file_logger {
            logger.log_info(&format!("Lithium Cache initialized with token: {}", token_config.token));
        }
    }
    
    let cache = Arc::new(Cache::new());
    let api_handler = Arc::new(ApiHandler::new(cache.clone(), token_config.token.clone()));
    
    let file_logger_for_shutdown = file_logger.clone();
    
    tokio::select! {
        _ = async {
            loop {
                match listener.accept().await {
                    Ok((socket, addr)) => {
                        let silent = config.silent;
                        let api_handler_clone = api_handler.clone();
                        let file_logger_clone = file_logger.clone();
                        tokio::spawn(async move {
                            if let Err(e) = handle_client(socket, silent, api_handler_clone, file_logger_clone).await {
                                if !silent {
                                    error!("Error handling client {}: {}", addr, e);
                                }
                            }
                        });
                    }
                    Err(e) => {
                        if !config.silent {
                            error!("Error accepting connection: {}", e);
                            if let Some(ref logger) = file_logger {
                                logger.log_error(&format!("Error accepting connection: {}", e));
                            }
                        }
                    }
                }
            }
        } => {},
        _ = signal::ctrl_c() => {}
    }
    
    if let Some(logger) = file_logger_for_shutdown {
        let _ = logger.shutdown();
    }
    
    Ok(())
}

async fn handle_client(mut socket: tokio::net::TcpStream, silent: bool, api_handler: Arc<ApiHandler>, file_logger: Option<Arc<FileLogger>>) -> io::Result<()> {
    use tokio::io::{AsyncReadExt, AsyncWriteExt};
    
    let mut buffer = [0; 1024];
    
    loop {
        match socket.read(&mut buffer).await {
            Ok(0) => {
                break;
            }
            Ok(n) => {
                let received = String::from_utf8_lossy(&buffer[..n]);
                let request = received.trim();
                
                let response = api_handler.handle_request(request);
                
                if !silent {
                    info!("Request: {}", request);
                    info!("Response: {}", response);
                }
                
                if let Some(ref logger) = file_logger {
                    logger.log_request(request);
                    logger.log_response(&response);
                }
                
                let response_with_newline = format!("{}\n", response);
                socket.write_all(response_with_newline.as_bytes()).await?;
            }
            Err(e) => {
                if !silent {
                    error!("Error reading from socket: {}", e);
                }
                if let Some(ref logger) = file_logger {
                    logger.log_error(&format!("Error reading from socket: {}", e));
                }
                break;
            }
        }
    }
    
    Ok(())
}
