#!/usr/bin/env python3
# spark_linear_regression.py
# Complete Spark implementation of linear regression with data generation

from pyspark.sql import SparkSession
from pyspark.sql.functions import rand, randn, col, lit
import pyspark.sql.functions as F
from pyspark.ml.regression import LinearRegression
from pyspark.ml.feature import VectorAssembler
import argparse

def generate_data(spark, num_samples, num_features, noise_std=0.1, seed=42):
    """
    Generate random data for linear regression with specified characteristics
    
    Args:
        spark: SparkSession object
        num_samples: Number of samples to generate
        num_features: Number of features per sample
        noise_std: Standard deviation of Gaussian noise
        seed: Random seed for reproducibility
    
    Returns:
        Spark DataFrame containing generated data
    """
    print(f"Generating {num_samples} samples with {num_features} features each...")
    
    # Create base DataFrame with IDs
    df = spark.range(num_samples)
    
    # Add feature columns (uniformly distributed between 0 and 100)
    for i in range(num_features):
        df = df.withColumn(f"feature_{i}", rand(seed=seed+i) * 100)
    
    # Calculate the true label (sum of features weighted by 1, 2, ..., num_features)
    label_expr = "0.0"
    for i in range(num_features):
        label_expr += f" + (feature_{i} * {i+1})"
    
    # Add Gaussian noise
    df = df.withColumn("noise", randn(seed=seed) * noise_std)
    
    # Calculate final label (true value + noise)
    df = df.withColumn("label", F.expr(label_expr) + col("noise"))
    
    # Drop temporary columns
    df = df.drop("noise", "id")
    
    return df

def train_model(df, num_features):
    """
    Train a linear regression model on the input data
    
    Args:
        df: Input DataFrame containing features and labels
        num_features: Number of features in the data
    
    Returns:
        Trained LinearRegressionModel and training summary
    """
    print("Preparing data for training...")
    
    # Prepare feature column names
    feature_cols = [f"feature_{i}" for i in range(num_features)]
    
    # Assemble features into a vector column
    assembler = VectorAssembler(
        inputCols=feature_cols,
        outputCol="features")
    
    assembled_data = assembler.transform(df)
    
    print("Training linear regression model...")
    
    # Create and configure linear regression model
    lr = LinearRegression(
        featuresCol="features",
        labelCol="label",
        maxIter=10,
        regParam=0.0,      # No regularization
        elasticNetParam=0.0, # Pure linear regression
        solver="normal",    # Normal equation solver
        standardization=False # Don't standardize features
    )
    
    # Train the model
    model = lr.fit(assembled_data)
    
    return model

def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser(description="Spark Linear Regression")
    parser.add_argument("--samples", type=float, default=19200000,
                       help="Number of samples to generate")
    parser.add_argument("--features", type=int, default=16,
                       help="Number of features per sample")
    parser.add_argument("--noise", type=float, default=0.1,
                       help="Standard deviation of Gaussian noise")
    parser.add_argument("--seed", type=int, default=42,
                       help="Random seed for reproducibility")
    args = parser.parse_args()
    
    # Initialize Spark session
    spark = SparkSession.builder \
        .appName("LinearRegressionExample") \
        .config("spark.driver.memory", "16g") \
        .config("spark.executor.memory", "16g") \
        .getOrCreate()
    
    try:
        # Step 1: Generate data
        data_df = generate_data(
            spark,
            num_samples=args.samples,
            num_features=args.features,
            noise_std=args.noise,
            seed=args.seed
        )
        
        # Cache the data for better performance
        data_df.cache()
        print(f"Data generation complete. Dataset size: {data_df.count()} rows")
        
        # Step 2: Train model
        model = train_model(data_df, args.features)
        
        # Step 3: Display results
        print("\nRegression Results:")
        print(f"Intercept: {model.intercept:.6f}")
        
        coefficients = model.coefficients
        for i, coef in enumerate(coefficients):
            print(f"Feature {i} coefficient: {coef:.6f} (expected: {i+1})")
        
        # Model evaluation
        training_summary = model.summary
        print("\nModel Evaluation:")
        print(f"RMSE: {training_summary.rootMeanSquaredError:.6f}")
        print(f"R²: {training_summary.r2:.6f}")
        print(f"Explained variance: {training_summary.explainedVariance:.6f}")
        
    finally:
        # Stop Spark session
        spark.stop()
        print("Spark session stopped")

if __name__ == "__main__":
    main()