# Graph Report - /mnt/d/temp/paul_class/project  (2026-05-29)

## Corpus Check
- Corpus is ~18,490 words - fits in a single context window. You may not need a graph.

## Summary
- 48 nodes · 51 edges · 8 communities (4 shown, 4 thin omitted)
- Extraction: 94% EXTRACTED · 6% INFERRED · 0% AMBIGUOUS · INFERRED: 3 edges (avg confidence: 0.75)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- [[_COMMUNITY_Plugin Foundation + Build|Plugin Foundation + Build]]
- [[_COMMUNITY_Data Fetching + Space Weather|Data Fetching + Space Weather]]
- [[_COMMUNITY_AudioProcessor Code (AST)|AudioProcessor Code (AST)]]
- [[_COMMUNITY_Project Scaffolding Toolchain|Project Scaffolding Toolchain]]
- [[_COMMUNITY_AudioEditor Code (AST)|AudioEditor Code (AST)]]
- [[_COMMUNITY_SynthParamMapper + Harmony|SynthParamMapper + Harmony]]
- [[_COMMUNITY_Cross-platform Validation|Cross-platform Validation]]
- [[_COMMUNITY_Docs + Release|Docs + Release]]

## God Nodes (most connected - your core abstractions)
1. `SynthParamMapper` - 6 edges
2. `DataFetcher` - 5 edges
3. `Phase 06 — Integration + Full UI` - 5 edges
4. `GeoTick UTC Grid API` - 5 edges
5. `SolarDroneAudioProcessor` - 4 edges
6. `AdditiveEngine` - 4 edges
7. `SynthParams` - 4 edges
8. `Phase 02 — DataFetcher` - 4 edges
9. `Project Scaffolder Skill` - 4 edges
10. `Interpolator` - 3 edges

## Surprising Connections (you probably didn't know these)
- `SolarDroneAudioProcessor` --semantically_similar_to--> `AdditiveEngine`  [INFERRED] [semantically similar]
  src/PluginProcessor.h → PROJECT.md
- `DataFetcher` --semantically_similar_to--> `GeoTick GPS Bridge`  [INFERRED] [semantically similar]
  PROJECT.md → facilitator/skill/project-scaffolder/references/geotick-example/PROJECT.md
- `SynthParamMapper` --semantically_similar_to--> `GeoTick UTC Grid API`  [INFERRED] [semantically similar]
  PROJECT.md → facilitator/skill/project-scaffolder/references/geotick-example/PROJECT.md
- `SolarDrone CMake Plugin Target` --references--> `SolarDroneAudioProcessor`  [EXTRACTED]
  CMakeLists.txt → src/PluginProcessor.h
- `SolarDrone CMake Plugin Target` --references--> `SolarDroneAudioProcessorEditor`  [EXTRACTED]
  CMakeLists.txt → src/PluginEditor.h

## Hyperedges (group relationships)
- **Core Data-to-Sound Pipeline** — project_datafetcher, project_synthparammapper, project_interpolator, project_additiveengine, project_visualrenderer [EXTRACTED 0.95]
- **PAUL Toolchain Artifact Generators** — facilitator_prompt_phases, skill_md_project_scaffolder, playbook_md_question_banks, templates_md_artifact_structures [EXTRACTED 0.95]
- **GeoTick Reference Example Bundle** — geotick_project_utc_grid_api, geotick_roadmap_phases, geotick_acceptance_criteria, geotick_usage_guide [EXTRACTED 0.95]

## Communities (8 total, 4 thin omitted)

### Community 0 - "Plugin Foundation + Build"
Cohesion: 0.23
Nodes (12): AdditiveEngine Acceptance Criteria, SolarDrone CMake Plugin Target, SolarDroneAudioProcessorEditor, createPluginFilter, SolarDroneAudioProcessor, AdditiveEngine, Interpolator, SynthParams (+4 more)

### Community 1 - "Data Fetching + Space Weather"
Cohesion: 0.2
Nodes (10): DataFetcher Acceptance Criteria, GeoTick GPS Bridge, GeoTick Roadmap Phases, GeoTick Usage Guide, DataFetcher, NOAA SWPC API, SpaceWeatherState, Phase 02 — DataFetcher (+2 more)

### Community 3 - "Project Scaffolding Toolchain"
Cohesion: 0.33
Nodes (7): Facilitator Prompt Phases, Project Scaffolder Facilitator, GeoTick Acceptance Criteria, GeoTick UTC Grid API, Scaffolder Playbook Question Banks, Project Scaffolder Skill, Artifact Templates

### Community 5 - "SynthParamMapper + Harmony"
Cohesion: 0.5
Nodes (4): SynthParamMapper Acceptance Criteria, SynthParamMapper, UserParams, Phase 03 — SynthParamMapper

## Knowledge Gaps
- **15 isolated node(s):** `createPluginFilter`, `UserParams`, `NOAA SWPC API`, `Phase 05 — VisualRenderer`, `Phase 07 — Cross-platform validation` (+10 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **4 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `SynthParamMapper` connect `SynthParamMapper + Harmony` to `Plugin Foundation + Build`, `Data Fetching + Space Weather`, `Project Scaffolding Toolchain`?**
  _High betweenness centrality (0.219) - this node is a cross-community bridge._
- **Why does `GeoTick UTC Grid API` connect `Project Scaffolding Toolchain` to `Data Fetching + Space Weather`, `SynthParamMapper + Harmony`?**
  _High betweenness centrality (0.174) - this node is a cross-community bridge._
- **Why does `Phase 06 — Integration + Full UI` connect `Plugin Foundation + Build` to `Data Fetching + Space Weather`, `SynthParamMapper + Harmony`?**
  _High betweenness centrality (0.173) - this node is a cross-community bridge._
- **What connects `createPluginFilter`, `UserParams`, `NOAA SWPC API` to the rest of the system?**
  _15 weakly-connected nodes found - possible documentation gaps or missing edges._