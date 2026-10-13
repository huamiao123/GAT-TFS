# Hot-source weighted partition audit, 2026-10-08

Adds a hot-source order: each destination row is anchored by the source neighbor with the largest source degree, then rows are sorted by that anchor. Weighted source duplication counts repeated AVX neighbor work. This is a structural hypothesis test, not a runtime claim.