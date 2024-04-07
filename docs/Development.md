
```ps1
$env:GRPC_EXPERIMENTS="event_engine,event_engine_client,event_engine_dns"

$env:GRPC_TRACE="event_engine,event_engine_endpoint"

$env:GRPC_VERBOSITY="debug"

$env:GRPC_DNS_RESOLVER="native"

```

Status:
Server Dns Resolver for event engine is not yet implemented in grpc core.
Chttp2ServerAddPort still uses none event engine resolver.