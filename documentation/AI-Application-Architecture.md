# ML Application Architecture

> NOTES:
>
> - This is a proposal that needs discussion
> - The term ML is used as ExecuTorch is a specialized ML inference runtime for edge deployment, not a full-stack AI runtime environment.

This is an attempt to harmonize how ML (example) applications could be structured.
It is based on the experience with SDS and MLEK, but should be generic enough to support also MLOps systems such as ModelNova.

Overall it should solve:

- How are ML models that are generated using LiteRT or ExecuTorch integrated in a MCU application
    - It should be irrelevant if the target is SDS or real-world application.
    - It should be possible to generate the ML part with an MLOps system or manually
- The target environment should allow to run on hardware or simulation models
    - I/O interfaces should use general streaming interfaces such as [CMSIS-Driver vstream](https://arm-software.github.io/CMSIS_6/latest/Driver/group__vstream__interface__gr.html)
    - Existing CMSIS board layers support such a structure already ([Simulation](https://github.com/ARM-software/SDS-Framework/tree/main/template/Board), [Boards](https://github.com/alifsemi/alif_ensemble-cmsis-dfp/tree/main/Boards))
- RTOS (FreeRTOS, RTX), bare-metal, Zephyr, and MicroPython deployment
    - Current framework fits well for FreeRTOS, RTX, and with small modifications even bare-metal deployments
    - For Zephyr, the ML layer should be mapped to an [external module](https://docs.zephyrproject.org/latest/samples/modules/index.html); related I/O driver interfaces may need to be developed.
    - For MicroPython we should engage with OpenMV
- How are ML run-time environments delivered to CMSIS or Zephyr based projects
    - For CMSIS based projects, a software pack is the best method. Potentially current software packs should be reworked.
    - For Zephyr based projects, including the framework in external module could be best approach.
- How are MLOps systems obtain parameters of the target system, configuration parmeters for Vela, and deliver ML models for deployment
    - CMSIS-Packs contain device features, however the [NPU feature](https://open-cmsis-pack.github.io/Open-CMSIS-Pack-Spec/main/html/pdsc_family_pg.html#element_feature) is not consistently applied.
    - Are there any other Ethos-U driver aspects that need to be reflected?
    - MLOps systems should allow configuration for Vela operation (size, speed optimization, etc.)
    - MLOps systems should directly deliver AI layer for CMSIS-based projects, Zephyr external module, tbd for MicroPython.
        - Vela report should be part of the deployment information
- Describe input and output streams for ML models (potentially applicable to Model Zoos and MLOps partners)
    - [SDS Metadata](https://arm-software.github.io/SDS-Framework/main/theory.html#yaml-metadata-format) could be used to describe input and output streams of ML Models
  
## Algorithm Integration

Pseudocode of a running an ML algorithm on a target (hardware or simulation):

```c
void AlgorithmThread() {
  InitEnvironment();     // Initialize for Input/Output interfaces
  InitAlgorithm();       // Initialize for ExecuteAlgorithm processing

  for (;;) {
    GetInputData(in_buf, sizeof(in_buf));
    // SDS input capturing here (as ExecuteAlgorithm may change in_buf)
    ExecuteAlgorithm(in_buf, sizeof(in_buf), out_buf, sizeof(out_buf));
    // SDS output capturing here (as ProcessOutputData may change out_buf)
    ProcessOutputData(out_buf, sizeof (outbuf);
  }
}
```

Notes:

- `ExecuteAlgorithm` may change `in_buf`. For example object recognition may add boxes around objects in the `in_buf` that represents the camera image.

### Algorithm Execution

The algorithm execution is specific to the runtime system (LiteRT or ExecuTorch).

Pseudocode of ExecuTorch:

```cpp
static RunnerContext *ctx = nullptr;

int32_t InitAlgorithm() {
    /* ---- Model Loading ---- */
    size_t pte_size = sizeof(model_pte);
    /* Construct BufferDataLoader in-place (no heap allocation) */
    loader_ptr = new (loader_storage) BufferDataLoader(model_pte, pte_size);

    /* Load the ExecuTorch program in-place */
    auto *program_result = new (program_result_storage)
        Result<Program>(Program::load(loader_ptr));
    if (!program_result->ok()) return -1;    // Error occured

    program_ptr = &program_result->get();

    /* ---- Runner Init (loads model method into RunnerContext) ---- */
    ctx = runner_context_instance();
    std::vector<std::pair<char *, size_t>> input_buffers; /* empty on init */
    runner_init(*ctx, input_buffers, pte_size, program_ptr);
    return 0;
}

int32_t ExecuteAlgorithm(uint8_t *in_buf, uint32_t in_num,
                         uint8_t *out_buf, uint32_t out_num) {

    /* Clear output buffer */
    memset(out_buf, 0, out_num);
    preprocess(in_buf);
    if (!run_inference(*ctx)) {
        return -1;                         // Error occured
    }
    postprocess(*ctx, in_buf, out_buf, out_num);
    return 0;
}
```

Pseudocode of LiteRT:

Todo

## Software Layers

The applications should be structured similar to [ModelNova example](https://github.com/Arm-Examples/ModelNova/tree/main/RockPaperScissors/AppKit-E8_USB) with the following software layers:

Layer Type       | Content
:----------------|-----------------------------------
ML               | ML Model, ML Runtime, Algorithm Execution
Board            | Board HAL interface
SDSIO            | SDSIO interface (when SDS-Framework is used)

The remaining parts of the application are directly in the *csolution project*.

## Software Packs

The software packs should be aligned with Zephyr modules. Ideally both the Zephyr module and the software pack is derived from the same repo. CMSIS-DSP might be a starting point.

## Zephyr Modules

ToDo: Zephry [external module](https://docs.zephyrproject.org/latest/samples/modules/index.html) sample for LiteRT and ExecuTorch.

## MLOps Integration

Todo

### Ethos Configuration

The table below lists possible Ethos-U configurations. For validation in an MLOps system, there should be a pre-configured SDS csolution project for functional ML model testing that contains the setup of the FVP simulation models.

Target Type      | MAC | System config
:----------------|-----|:------------------------------
SSE-320-U85-2048 | 2048| Ethos-U85, Armv8.1M (Corstone-320)
SSE-320-U85-1024 | 1024| Ethos-U85, Armv8.1M (Corstone-320)
SSE-320-U85-512  | 512 | Ethos-U85, Armv8.1M (Corstone-320)
SSE-320-U85-256  | 256 | Ethos-U85, Armv8.1M (Corstone-320)
SSE-320-U85-128  | 128 | Ethos-U85, Armv8.1M (Corstone-320)
SSE-315-U65-512  | 512 | Ethos-U65, Armv8.1M (Corstone-315)
SSE-315-U65-256  | 256 | Ethos-U65, Armv8.1M (Corstone-315)
SSE-300-U55-256  | 256 | Ethos-U55, Armv8.1M (Corstone-300)
SSE-300-U55-128  | 128 | Ethos-U55, Armv8.1M (Corstone-300)

Note: Armv8.1M represents the instruction set used in Cortex-M52, M55, M85 and Star-MC2, MC3. FVP are not timing accurate, therefore the exact processor configuration should not matter to an functional validation flow in MLOps.