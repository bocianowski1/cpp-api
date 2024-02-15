FROM golang:1.20 AS builder

WORKDIR /app

COPY . .

COPY go.mod ./
RUN go mod download

RUN CGO_ENABLED=0 GOOS=linux go build -o server

FROM alpine:latest

WORKDIR /app

COPY --from=builder /app/server .

CMD ["./server"]
