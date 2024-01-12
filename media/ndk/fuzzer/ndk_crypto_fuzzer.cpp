/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fuzzer/FuzzedDataProvider.h>
#include <media/NdkMediaCrypto.h>
#include <functional>

#include <functional>

constexpr size_t kMaxString = 256;
constexpr size_t kMinBytes = 0;
constexpr size_t kMaxBytes = 1000;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider fdp(data, size);
    AMediaUUID uuid = {};
    int32_t maxLen = fdp.ConsumeIntegralInRange<size_t>(kMinBytes, (size_t)sizeof(AMediaUUID));
    for (size_t idx = 0; idx < maxLen; ++idx) {
        uuid[idx] = fdp.ConsumeIntegral<uint8_t>();
    }
    std::vector<uint8_t> initData =
            fdp.ConsumeBytes<uint8_t>(fdp.ConsumeIntegralInRange<size_t>(kMinBytes, kMaxBytes));
    AMediaCrypto* crypto = AMediaCrypto_new(uuid, initData.data(), initData.size());
    while (fdp.remaining_bytes()) {
        auto invokeNdkCryptoFuzzer = fdp.PickValueInArray<const std::function<void()>>({
                [&]() {
                    AMediaCrypto_requiresSecureDecoderComponent(
                            fdp.ConsumeRandomLengthString(kMaxString).c_str());
                },
                [&]() { AMediaCrypto_isCryptoSchemeSupported(uuid); },
        });
        invokeNdkCryptoFuzzer();
    }
    AMediaCrypto_delete(crypto);
    return 0;
}
