# Examples

| File | What it shows | Built by `make test` |
|---|---|---|
| `host_quickstart.c` | The first-screen code from the guide, running against the PC controller model | yes, built and run |
| `port_fmc.c` | Port for a 16-bit FMC with `FMC_A0` on D/CX | yes, compiled with strict warnings |
| `port_spi.c` | Port for 4-wire SPI, tracking the last command | yes, compiled with strict warnings |
| `port_8bit.c` | Port for an 8-bit parallel bus | yes, compiled with strict warnings |
| `cubeide_main_snippet.c` | Where the self-test and demo loops go in a CubeIDE `main.c` | no, it needs the HAL |

Run the host example:

```sh
make examples
build/host_quickstart build/first.ppm
```

The three port files define the same two functions, so use exactly one of them in a project.
