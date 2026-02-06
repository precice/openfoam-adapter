# Implicit coupling diagram

```mermaid
flowchart TD
    subgraph loop ["Implicit Iteration Loop for Window t"]
        direction TB
        B{"OpenFOAM Solve"}
        C["Adapter::execute()"]
        D["Adapter:writeCouplingData()"]
        E["preCICE::advance()"]
        F{"preCICE - Rollback Needed?"}
        G["Adapter::readCheckpoint() (Rollback)"]
        I["Adapter - Read NEW Coupled Solver Data"]

        J{"preCICE - Save State Needed?"}
        K["Adapter::writeCheckpoint()"]
        L["Adapter - Read FINAL/Next Window Data"]

    end

    B --> C;
    C --> D;
    D --> E;
    E --> F;

    F -- Yes --> G;
    G --> I;
    I --> B;

    F -- No --> J; %% Converged or explicit
    K --> L;
    J -- Yes --> K;
    J -- No --> L;

    L --> M["OpenFOAM - Advance to Next Window t+dt"];

    %% Styles
    style B fill:#f9f,stroke:#333  %% Solver Solve
    style M fill:#f9f,stroke:#333  %% Solver Advance Window

    style C fill:#eef,stroke:#333  %% Adapter::execute() entry

    style D fill:#ffdacc,stroke:#333  %% Write Coupling Data
    style K fill:#ffdacc,stroke:#333  %% Write Checkpoint

    style E fill:#eee,stroke:#333  %% preCICE advance
    style F fill:#eee,stroke:#333,stroke-dasharray: 5 5 %% preCICE Rollback Check
    style J fill:#eee,stroke:#333,stroke-dasharray: 5 5 %% preCICE Save Check

    style G fill:#cfc,stroke:#333  %% Read Checkpoint
    style I fill:#cfc,stroke:#333  %% Read NEW Data
    style L fill:#cfc,stroke:#333  %% Read FINAL/Next Data

    style loop stroke:#aaa,stroke-width:1px,stroke-dasharray: 3
```
