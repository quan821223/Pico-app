import hashlib
import json
import os
import re
import subprocess
import unittest
from pathlib import Path


CONTRACT_PATH = Path(__file__).with_name("protocol_vectors.json")
EXPECTED_VECTOR_SHA256 = "651a8cc07078f32f7cb831f973b6156c9f29b46b681d17773e8183ed244b9ee1"
HEX_BYTES = re.compile(r"^(?:[0-9A-F]{2})(?: [0-9A-F]{2})*$")
PATTERN_BYTES = re.compile(r"^(?:[0-9A-F?]{2})(?: [0-9A-F?]{2})*$")


class ProtocolContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.contract = json.loads(CONTRACT_PATH.read_text(encoding="utf-8"))
        cls.vectors = cls.contract["vectors"]

    def test_contract_is_tied_to_refactoring_baseline(self):
        self.assertEqual(
            self.contract["baseline_commit"],
            "8d3920fcb38ba616baee897c2b64b841a295dd85",
        )
        self.assertEqual(self.contract["default_response_delay_ms"], 50)

    def test_names_and_requests_are_unique(self):
        names = [vector["name"] for vector in self.vectors]
        requests = [vector["request"] for vector in self.vectors]
        self.assertEqual(len(names), len(set(names)))
        self.assertEqual(len(requests), len(set(requests)))

    def test_every_request_is_a_five_byte_fa_or_da_frame(self):
        for vector in self.vectors:
            with self.subTest(vector=vector["name"]):
                request = vector["request"]
                self.assertRegex(request, HEX_BYTES)
                request_bytes = bytes.fromhex(request)
                self.assertEqual(len(request_bytes), 5)
                self.assertIn(request_bytes[0], (0xFA, 0xDA))
                self.assertIn(request_bytes[1], (0x52, 0x57))

    def test_responses_have_valid_contract_encoding(self):
        for vector in self.vectors:
            with self.subTest(vector=vector["name"]):
                if "response_pattern" in vector:
                    self.assertRegex(vector["response_pattern"], PATTERN_BYTES)
                    self.assertTrue(vector["response_pattern"].endswith("0D 0A"))
                    continue
                response = vector.get("response")
                if response is None:
                    continue
                self.assertRegex(response, HEX_BYTES)
                response_bytes = bytes.fromhex(response)
                self.assertGreaterEqual(len(response_bytes), 3)
                self.assertEqual(response_bytes[-2:], b"\r\n")

    def test_reviewed_vector_set_has_not_changed_silently(self):
        canonical = json.dumps(
            self.vectors, ensure_ascii=False, sort_keys=True, separators=(",", ":")
        ).encode("utf-8")
        self.assertEqual(hashlib.sha256(canonical).hexdigest(), EXPECTED_VECTOR_SHA256)

    def test_production_command_logic_matches_reviewed_vectors(self):
        runner = os.environ.get("PROTOCOL_CONTRACT_RUNNER")
        if not runner:
            self.skipTest("PROTOCOL_CONTRACT_RUNNER is not configured")

        for vector in self.vectors:
            with self.subTest(vector=vector["name"]):
                request_bytes = vector["request"].split()
                completed = subprocess.run(
                    [runner, *request_bytes, "02"],
                    check=True,
                    capture_output=True,
                    text=True,
                )
                actual = completed.stdout.strip()

                if "response_pattern" in vector:
                    expected = vector["response_pattern"].replace("??", "02")
                elif vector.get("response") is None:
                    expected = "NONE"
                else:
                    expected = vector["response"]

                self.assertEqual(actual, expected)


if __name__ == "__main__":
    unittest.main()
