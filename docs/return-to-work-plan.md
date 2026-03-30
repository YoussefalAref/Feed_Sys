# Feed_Sys Return-to-Work Plan

This document is the persistent execution playbook for this repository.
When you come back to work, start here instead of re-planning from scratch.

## 1) Session Startup Checklist (Always Run)

1. Confirm branch and local state.
   - `git status`
   - `git branch --show-current`
2. Re-open this file and check the milestone board in section 4.
3. Build sanity check.
   - Backend: compile main target (CMake or g++)
   - Frontend: `npm install` (if needed), then `npm run build`
4. Pick exactly one active milestone for the session.
5. Run the resume prompt in section 6.

## 2) Quota-Efficient Working Rules

- Work in one milestone at a time; do not split focus.
- Prefer one implementation pass per milestone:
  - inspect -> edit -> validate -> summarize.
- Require strict update format from Copilot:
  - files changed
  - commands run
  - test/build result
  - next smallest step
- Avoid asking for alternative solutions unless blocked.
- Defer non-critical polish until core flow is complete.

## 3) Project End-State (Definition of Complete)

The project is complete when all are true:

- C++ data structures and recommendation flow are correct and tested.
- Backend exposes API endpoints for auth, products, recommendations, interactions.
- Data persists across restarts (at least one supported persistence mode).
- Frontend uses live backend APIs (no static-only fallback for core paths).
- Interaction events update recommendations.
- Basic auth validation/security is in place.
- CI validates build + tests.
- Docs include architecture, API contract, run steps, demo scenario.

## 4) Milestone Board

Mark status as: `todo`, `doing`, `done`, `blocked`.

| # | Milestone | Status | Exit Criteria |
|---|---|---|---|
| 1 | Stabilize repository baseline | todo | Backend/frontend build reliably; no unknown blockers |
| 2 | Finalize C++ data structures | todo | HashMap/Heap/Queue/Graph verified with edge cases |
| 3 | Build recommendation service layer | todo | One clear service API for ranking + similarity + updates |
| 4 | Expose backend HTTP API | todo | Frontend can fetch products/recommendations and send interactions |
| 5 | Add persistence and seed data | todo | Data survives restart and can be seeded reproducibly |
| 6 | Integrate frontend with API | todo | Sign-in, product list, cart, recommendation flows run on live API |
| 7 | Implement interaction telemetry flow | todo | User actions alter recommendation outputs |
| 8 | Harden auth and validation | todo | Protected flows + payload validation enforced |
| 9 | Add tests and CI checks | todo | Automated tests and CI pass before merge |
| 10 | Prepare docs, demo, release | todo | Runbook + API docs + deterministic demo available |

## 5) Per-Session Log Template

Add one entry each time you stop working:

```
Date:
Milestone:
What changed:
Validation run:
Current blocker (if any):
Next first action:
```

## 6) Copy/Paste Resume Prompt

Use this exact prompt when returning:

```
Continue Feed_Sys using docs/return-to-work-plan.md as the source of truth.
1) Run startup checklist quickly.
2) Tell me current milestone status from the board.
3) Execute the next smallest step for the active milestone.
4) Keep output compact: changed files, commands, validation, next step.
5) Do not re-plan the whole project unless blocked.
```

## 7) Blocker Protocol

If blocked for more than 20 minutes:

1. Record blocker in session log.
2. Propose up to 2 practical alternatives.
3. Choose one and continue immediately.

This prevents stall loops and repeated quota burn.
