<html>
	<head>
		<title>BSU Cafeteria Simulation</title>
	</head>
	<body>
		<h3>BSU Cafeteria Simulation</h3>
		<h4>
			by:
			<br/>13513018 Steven Andianto
			<br/>13513077 Calvin Aditya Jonathan
		</h4>
		<hr>
		<p>
			<form method="post">
				 Select case number:
				 <br/>
				 <br/>
				 <select name="case">
				 	<option value="0">Base case</option>
				  	<option value="1">Case A-I</option>
				  	<option value="2">Case A-II</option>
				  	<option value="3">Case A-III</option>
				  	<option value="4">Case B-I</option>
				  	<option value="5">Case B-II</option>
				  	<option value="6">Case B-III</option>
				  	<option value="7">Case C</option>
				</select>
				<br/>
				<br/>
				<input type="submit" value="Run simulation">
			</form>
		</p>

		<?php
			if (isset($_POST['case'])) {
				$case = $_POST['case'];
				echo '<hr>';
				echo 'Simulation result:<br/><br/>';
				echo '<pre>';
				passthru("./bsu $case");
				echo '</pre>';
			}
		?>
	</body>
</html>