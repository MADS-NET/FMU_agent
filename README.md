# FMU Agent for MADS

This repository Produces a monolithic [MADS](https://github.com/pbosetti/mads) agent that can run a simulation model (a `.fmu` file) in [FMI3](https://fmi-standard.org/docs/3.0/) format. 

Functional Mockup Units (`.fmu` files) can be exported from a large number of simulation softwares like MATLAB Simulink.

> **NOTE:** Only the last format of FMI format, i.e. FMI 3.0, is supported.

## Build

As usual for MADS plugins and agents:

```sh
cmake -Bbuild
cmake --build build -j6
```

A few notes:

- The default install prefix is the MADS folder (as for `mads -p`)
- The option `MADS_INSTALL_AGENT` (default: off) enables installation in the prefix directory of the `mads-fmu` agent, so that you can call it as `mads fmu`
- The option `MADS_BUILD_FMU` (default: off) enables compilation and creation of the FMU units from the source files available in the `models` directory; those are intended for testing purposes


## FMUs

Currently, only **model exchange** FMUs are supported. Co-simulation FMUs are not supported.


## Execution

Once the agent is compiled and you have a valid FMI3.0 compatible `my_model.fmu` file, you shall chek it for the actual naming of internal variables. Assuming that the agent has been installed (with `cmake --install build`), and that the FMU file is in your current working directory:

```sh
mads fmu my_model.fmu --inspect
```

This is giving you a table of model variables and a suitable stub section for the `mads.ini` file. Copy that into your settings file, update the fields as needed and launch the `mads broker` command, if it is not running already.

Now you can run the agent:

```sh
mads fmu my_model.fmu
```

Note that by default the model runs at 100 ms period. You can change that either via the `period` value in the INI section, or override that via the `-p|--period` command line option.

The model is routinely evaluated and forward integrated at variable timestep, and the new status is published on the `pub_topic`. At any time, a **new input** can be gived via the MADS network, by publising to the `sub_topic` a JSON message as:

```json
{
  "fmu_input": {
    "in_var1": 123.0,
    "in_var2": 0.123
  }
}
```

It is suggested to use the [rerunner plugin](https://github.com/mads-net/rerunner_plugin) to visualize the model state in real time.

> **NOTE:** the model is suppsed to run in real time, i.e. the solution time for each timestep shall be shorter than the timestep itself. Relax the tolerances in the INI section if your model cannot keep up with the real time.