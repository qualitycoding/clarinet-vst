"""Phase 3.6 mechanical check (Haiku-role substitute; clarinet plan): IDs referenced vs defined; paths referenced vs present/created."""
import re, json, pathlib, sys
R = pathlib.Path(__file__).resolve().parents[2]
docs = [R/"HANDOFF.md", *sorted((R/"plan").glob("*.md")), R/"premortem/RISK_REGISTER.md"]
text = {p: p.read_text() for p in docs if p.exists()}
alltext = "\n".join(text.values())
defined = set()
defined |= set(re.findall(r"\*\*(D-\d{3})", (R/"plan/DECISIONS.md").read_text()))
defined |= set(re.findall(r"\| (A-\d{3}) \|", (R/"plan/ASSUMPTIONS.md").read_text()))
defined |= set(re.findall(r"### (S-\d{3})", (R/"plan/PLAN.md").read_text()))
defined |= set(re.findall(r"## (G-\d{3})", (R/"plan/GATES.md").read_text())) | {"G-001","G-002"}
defined |= {c["id"] for c in json.loads((R/"research/claims.json").read_text())}
defined |= set(re.findall(r"\| (SC-\d+) \|", (R/"plan/TRACEABILITY.md").read_text()))
tests = set(re.findall(r"\[(T-\d{3})\]", "\n".join(p.read_text() for p in (R/"tests").rglob("*.cpp"))))
tests |= set(re.findall(r"(T-\d{3})", "\n".join(p.read_text() for p in (R/"tests").rglob("*.py"))))
tests |= set(re.findall(r"(T-\d{3})", "\n".join(p.read_text() for p in (R/"tests/scripts").glob("*.sh"))))
defined |= tests | {t+s for t in tests for s in "ab"}
if (R/"premortem/RISK_REGISTER.md").exists():
    defined |= set(re.findall(r"\| (R-\d{3}) \|", (R/"premortem/RISK_REGISTER.md").read_text()))
refs = set(re.findall(r"\b([ADSGCRT]-\d{3}[ab]?|SC-\d+)\b", alltext))
missing = sorted(r for r in refs if r not in defined)
print("undefined IDs:", missing)
# tests in traceability vs tests in code
trace = set(re.findall(r"\| (T-\d{3}[ab]?) \|", (R/"plan/TRACEABILITY.md").read_text()))
print("tests in code not in traceability:", sorted(t for t in tests if t not in trace and t+"a" not in trace))
paths = set(re.findall(r"`((?:core|plugin|tools|tests|data|plan|research|premortem|gates|logs)/[\w./\-<>*{},]+)`", alltext))
created = "\n".join(re.findall(r"- Outputs: (.*)", (R/"plan/PLAN.md").read_text()))
bad = [p for p in sorted(paths) if not any(c in p for c in "<*{") and not (R/p).exists() and p not in created and p.split("/")[-1] not in created]
print("paths neither present nor produced by a step:", bad)
