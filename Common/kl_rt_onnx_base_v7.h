#ifndef KL_RT_BASE_H
#define KL_RT_BASE_H

#include "NvInfer.h"
#include <string>
#include <vector>
#include <cuda_runtime_api.h>
#include "kl_rt_tensor.hpp"
#include <iostream>

using namespace nvinfer1;

namespace algocomon
{
	//加载神经网络需要的配置
	struct OnnxNetInitParam
	{
		std::string onnx_model; //onnx_model路径
		bool security = false;	//onnx模型是否加密
		int max_batch_size = 1;
	};

	class KLRTOnnxBaseV7
	{
	public:
		KLRTOnnxBaseV7(const OnnxNetInitParam &param);
		KLRTOnnxBaseV7() = default;
		virtual ~KLRTOnnxBaseV7();

	public:
		void LoadOnnxModel(const std::string &onnx_file);

		bool loadGieStreamBuildContext(const std::string &gie_file);

		std::vector<KLTensorFloat> &Forward();

		std::vector<KLTensorFloat> &operator()();

		void SaveRtModel(const std::string &path);

		void checkCuda(cudaError error);

		void Release();

	public:
		inline KLTensorFloat &input_tensor()
		{
			return input_tensor_;
		}

		void set_max_batch_size(int max_batch_size)
		{
			max_batch_size_ = max_batch_size;
		}

		inline void set_batch_size(int batch_size)
		{
			batch_size_ = batch_size;
			if (batch_size_ > max_batch_size_)
			{
				batch_size_ = max_batch_size_;
			}
		}

		inline int batch_size() const
		{
			return batch_size_;
		}

		inline int max_batch_size() const
		{
			return max_batch_size_;
		}

		Shape input_shape() const
		{
			return input_shape_;
		}

		std::vector<Shape> output_shape() const
		{
			return output_shape_;
		}

		cudaStream_t &stream()
		{
			return stream_;
		}

	protected:
		class Logger : public nvinfer1::ILogger
		{
		public:
			void log(nvinfer1::ILogger::Severity severity, const char *msg) override
			{
				// suppress info-level messages
				if (severity == Severity::kINFO)
					return;

				switch (severity)
				{
				case Severity::kINTERNAL_ERROR:
					std::cerr << "INTERNAL_ERROR: " << msg << std::endl;
					break;
				case Severity::kERROR:
					std::cerr << "ERROR: " << msg << std::endl;
					break;
				case Severity::kWARNING:
					std::cerr << "WARNING: " << msg << std::endl;
					break;
				case Severity::kINFO:
					std::cerr << "INFO: " << msg << std::endl;
					break;
				default:
					// std::cerr << "UNKNOWN: " << msg << std::endl;
					break;
				}
			}
		};

	protected:
		inline void set_use_fp16(bool use)
		{
			use_fp16_ = use;
		}

		IHostMemory *gie_model_stream_ = nullptr;

		std::vector<void *> buffer_queue_;

		//cuda stream
		cudaStream_t stream_;

	private:
		void mallocInputOutput();

		bool SimpleXor(const std::string &info, const std::string &key, std::string &result);

		void GetFileString(const std::string &file, std::string &str);

		void deserializeCudaEngine(const void *blob, std::size_t size);

	private:
		Shape input_shape_;
		std::vector<Shape> output_shape_;

		//input output info
		KLTensorFloat input_tensor_;

		std::vector<KLTensorFloat> output_tensors_;

		//batch size
		int batch_size_ = 1;
		int max_batch_size_ = 1;
		//
		bool use_fp16_ = false;

		Logger logger_;
		IExecutionContext *context_ = nullptr;
		ICudaEngine *engine_ = nullptr;
		IRuntime *runtime_ = nullptr;
		bool configured_ = false;
		bool release_ = false;

		std::string encryp_key_ = "this is koala key";
	};
} // namespace algocomon

#endif