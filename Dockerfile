FROM rust:slim AS builder

RUN apt-get update && \
    apt-get install -y \
        ca-certificates \
        && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN cargo build --release --bin lithium-cache-service

FROM debian:bookworm-slim

RUN apt-get update && \
    apt-get install -y \
        ca-certificates \
        && \
    rm -rf /var/lib/apt/lists/*

RUN groupadd -r lithium-cache && useradd -r -g lithium-cache lithium-cache

WORKDIR /app

COPY --from=builder /app/target/release/lithium-cache-service /usr/local/bin/lithium-cache-service

RUN chown -R lithium-cache:lithium-cache /app

USER lithium-cache

EXPOSE 1227

ENTRYPOINT ["/usr/local/bin/lithium-cache-service"]